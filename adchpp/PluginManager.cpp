/*
 * Copyright (C) 2006-2018 Jacek Sieka, arnetheduck on gmail point com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include "adchpp.h"
#include "PluginManager.h"
#include "Core.h"
#include "LogManager.h"
#include "ScriptManager.h"

namespace adchpp
{

	using std::string;
	using std::function;
	using std::placeholders::_1;

	const string PluginManager::className = "PluginManager";

	PluginManager::PluginManager(Core& core) noexcept : core(core)
	{
	}

	void PluginManager::attention(const function<void()>& f)
	{
		core.addJob(f);
	}

	void PluginManager::load()
	{
		for (StringIter i = plugins.begin(); i != plugins.end(); ++i)
			if (*i == "Script")
			{
				auto sm = std::make_shared<ScriptManager>(getCore());
				sm->load();
				registerPlugin("ScriptManager", sm);
			}
			else
				LOG(className, "Unknown plugin '" + *i + "'");
	}

	void PluginManager::shutdown()
	{
		registry.clear();
	}

	PluginManager::CommandDispatch::CommandDispatch(PluginManager& pm, const std::string& name, const PluginManager::CommandSlot& f)
	: name('+' + name), f(f), pm(&pm)
	{
	}

	void PluginManager::CommandDispatch::operator()(Entity& e, AdcCommand& cmd, bool& ok)
	{
		if (e.getState() != Entity::STATE_NORMAL)
			return;

		if (cmd.getCommand() != AdcCommand::CMD_MSG)
			return;

		if (cmd.getParameters().size() < 1)
			return;

		StringList l;
		Util::tokenize(l, cmd.getParameters()[0], ' ');
		if (l[0] != name)
			return;
		l[0] = name.substr(1);
		if (!pm->handleCommand(e, l))
			return;

		cmd.setPriority(AdcCommand::PRIORITY_IGNORE);
		f(e, l, ok);
	}

	ClientManager::SignalReceive::Connection PluginManager::onCommand(const std::string& commandName, const CommandSlot& f)
	{
		return core.getClientManager().signalReceive().connect(CommandDispatch(*this, commandName, f));
	}

	PluginManager::CommandSignal& PluginManager::getCommandSignal(const std::string& commandName)
	{
		CommandHandlers::iterator i = commandHandlers.find(commandName);
		if (i == commandHandlers.end())
			return commandHandlers.insert(make_pair(commandName, CommandSignal())).first->second;

		return i->second;
	}

	bool PluginManager::handleCommand(Entity& e, const StringList& l)
	{
		CommandHandlers::iterator i = commandHandlers.find(l[0]);
		if (i == commandHandlers.end()) return true;

		bool ok = true;
		i->second(e, l, ok);
		return ok;
	}

	Core& PluginManager::getCore()
	{
		return core;
	}
} // namespace adchpp
