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

#include "Core.h"
#include "ClientManager.h"
#include "LogManager.h"
#include "PluginManager.h"
#include "SocketManager.h"
#include "version.h"
#include <baselib/File.h>

namespace adchpp
{
	using std::string;

	std::shared_ptr<Core> Core::create(const string& configPath)
	{
		auto ret = std::shared_ptr<Core>(new Core(configPath));
		ret->init();
		return ret;
	}

	Core::Core(const string& configPath) : configPath(configPath), startTime(time::now())
	{
#ifdef _WIN32
		dataPath = configPath;
#else
		dataPath = "/var/lib/" APPNAME "/data/";
#endif
	}

	Core::~Core()
	{
		lm->log("core", "Shutting down...");
		// Order is significant...
		pm.reset();
		cm.reset();
		sm.reset();
		lm.reset();
	}

	void Core::init()
	{
		lm.reset(new LogManager(*this));
		sm.reset(new SocketManager(*this));
		cm.reset(new ClientManager(*this));
		pm.reset(new PluginManager(*this));

		sm->setIncomingHandler(std::bind(&ClientManager::handleIncoming, cm.get(), std::placeholders::_1));
		// lm->log("core", "Core initialized"); @todo logfile path setting isn't
		// processed yet so this may litter log files to unwanted places, see L#907372
		// lm->log("core", "Core initialized");
		printf("\nCore initialized\n"); // Console print only for now...
	}

	void Core::run()
	{
		File::ensureDirectory(dataPath);
		pm->load();
		sm->run();
	}

	void Core::shutdown()
	{
		// make sure we run shutdown routines from the right thread.
		addJob(std::bind(&Core::doShutdown, this));
	}

	void Core::doShutdown()
	{
		sm->shutdown();
		pm->shutdown();
	}

	LogManager& Core::getLogManager()
	{
		return *lm;
	}

	SocketManager& Core::getSocketManager()
	{
		return *sm;
	}

	PluginManager& Core::getPluginManager()
	{
		return *pm;
	}

	ClientManager& Core::getClientManager()
	{
		return *cm;
	}

	void Core::addJob(const Callback& callback) noexcept
	{
		sm->addJob(callback);
	}

	void Core::addJob(const long msec, const Callback& callback)
	{
		sm->addJob(msec, callback);
	}

	void Core::addJob(const string& time, const Callback& callback)
	{
		sm->addJob(time, callback);
	}

	Core::Callback Core::addTimedJob(const long msec, const Callback& callback)
	{
		return sm->addTimedJob(msec, callback);
	}

	Core::Callback Core::addTimedJob(const string& time, const Callback& callback)
	{
		return sm->addTimedJob(time, callback);
	}

} // namespace adchpp
