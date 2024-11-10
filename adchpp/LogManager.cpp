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
#include "version.h"
#include <baselib/File.h>
#include <baselib/FormatUtil.h>
#include <baselib/PathUtil.h>
#include <baselib/ParamExpander.h>

using namespace adchpp;

LogManager::LogManager(Core& core) : enabled(true), useConsole(true), core(core)
{
#ifdef _WIN32
	logFileTemplate = "logs/%Y%m.log";
#else
	logFileTemplate = "/var/log/" APPNAME "/%Y%m.log";
#endif
}

void LogManager::log(const string& area, const string& msg) noexcept
{
	static const string format("%Y-%m-%d %H:%M:%S: ");
	Util::TimeParamExpander ex(::time(nullptr));
	string tmp = Util::formatParams(format, &ex, false);
	tmp += area;
	tmp += ": ";
	tmp += msg;
	if (tmp.back() != '\n') tmp += '\n';
	doLog(ex, tmp);
}

void LogManager::doLog(Util::ParamExpander& ex, const string& msg) noexcept
{
	sig(msg);
	if (useConsole)
	{
		fputs(msg.c_str(), stdout);
		fflush(stdout);
	}
	if (!enabled) return;

	string fileName = logFileTemplate;
	Util::toNativePathSeparators(fileName);
	string newFileName = Util::formatParams(AppPaths::makeAbsolutePath(core.getConfigPath(), fileName), &ex, false);
	LockBase<CriticalSection> l(mtx);
	try
	{
		if (newFileName != currentFileName)
		{
			currentFileName.clear();
			File::ensureDirectory(newFileName);
			file.close();
#ifdef _WIN32
			file.init(Text::utf8ToWide(newFileName), File::WRITE, File::OPEN | File::CREATE);
#else
			file.init(newFileName, File::WRITE, File::OPEN | File::CREATE);
#endif
			file.setEndPos(0);
			currentFileName = std::move(newFileName);
		}
		file.write(msg);
		return;
	}
	catch (const FileException& e)
	{
		dcdebug("LogManager::log: %s\n", e.getError().c_str());
		return;
	}
}
