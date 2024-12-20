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

#ifndef SCRIPT_MANAGER_H_
#define SCRIPT_MANAGER_H_

#include <baselib/Exception.h>
#include "ClientManager.h"
#include "Plugin.h"

namespace adchpp
{

STANDARD_EXCEPTION(ScriptException);

class Engine;
class Client;
class AdcCommand;

class ScriptManager : public Plugin
{
public:
	ScriptManager(Core& core);
	virtual ~ScriptManager();

	virtual int getVersion()
	{
		return 0;
	}

	void load();

	static const std::string className;

private:
	std::vector<std::unique_ptr<Engine>> engines;

	void reload();
	void clearEngines();

	ClientManager::SignalReceive::ManagedConnection reloadConn;
	ClientManager::SignalReceive::ManagedConnection statsConn;

	void onReload(Entity& c);
	void onStats(Entity& c);

	Core& core;
};

}

#endif // SCRIPT_MANAGER_H_
