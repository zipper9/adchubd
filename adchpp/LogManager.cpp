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

#include "LogManager.h"
#include "Core.h"
#include "AppPaths.h"
#include <baselib/File.h>
#include <baselib/FormatUtil.h>
#include <baselib/PathUtil.h>

namespace adchpp
{

	LogManager::LogManager(Core& core) : logFile("logs/adchubd/%Y%m.log"), enabled(true), core(core)
	{
	}

	void LogManager::log(const string& area, const string& msg) noexcept
	{
		char buf[64];
		time_t now = std::time(NULL);
		size_t s = strftime(buf, 64, "%Y-%m-%d %H:%M:%S: ", localtime(&now));
		string tmp(buf, s);
		tmp += area;
		tmp += ": ";
		tmp += msg;
		dolog(tmp);
	}

	void LogManager::dolog(const string& msg) noexcept
	{
		dcdebug("Logging: %s\n", msg.c_str());
		signalLog_(msg);
		if (getEnabled())
		{
			string fileName = getLogFile();
			Util::toNativePathSeparators(fileName);
			string logFilePath = Util::formatDateTime(AppPaths::makeAbsolutePath(core.getConfigPath(), fileName), ::time(nullptr));
			LockBase<CriticalSection> l(mtx);
			try
			{
				File f(logFilePath, File::WRITE, File::OPEN | File::CREATE);
				f.setEndPos(0);
				f.write(msg + "\r\n");
				return;
			}
			catch (const FileException& e)
			{
				dcdebug("LogManager::log: %s\n", e.getError().c_str());
			}
			try
			{
				File::ensureDirectory(logFilePath);
				File f(logFilePath, File::WRITE, File::OPEN | File::CREATE);
				f.setEndPos(0);
				f.write(msg + "\r\n");
			}
			catch (const FileException& ee)
			{
				dcdebug("LogManager::log2: %s\n", ee.getError().c_str());
			}
		}
	}

} // namespace adchpp
