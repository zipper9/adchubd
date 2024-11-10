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

#ifndef ADCHPP_LOGMANAGER_H
#define ADCHPP_LOGMANAGER_H

#include <baselib/Locks.h>
#include <baselib/File.h>
#include "Signal.h"

namespace Util { class ParamExpander; }

namespace adchpp
{
	class Core;

	/**
	 * Log writing utilities.
	 */
	class LogManager
	{
	public:
		/**
		 * Add a line to the log.
		 * @param area Name of the module that generated the error.
		 * @param msg Message to log.
		 */
		void log(const std::string& area, const std::string& msg) noexcept;

		void setLogFile(const std::string& s) { logFileTemplate = s; }
		const std::string& getLogFile() const { return logFileTemplate; }
		void setEnabled(bool flag) { enabled = flag; }
		bool getEnabled() const { return enabled; }
		void setUseConsole(bool flag) { useConsole = flag; }
		bool getUseConsole() const { return useConsole; }

		typedef SignalTraits<void(const std::string&)> SignalLog;
		SignalLog::Signal& signalLog() { return sig; }

	private:
		friend class Core;

		CriticalSection mtx;
		std::string logFileTemplate;
		std::string currentFileName;
		File file;
		bool enabled;
		bool useConsole;

		LogManager(Core& core);

		SignalLog::Signal sig;
		Core& core;

		void doLog(Util::ParamExpander& ex, const std::string& msg) noexcept;
	};

#define LOGC(core, area, msg) (core).getLogManager().log(area, msg)
#define LOG(area, msg) LOGC(core, area, msg)

} // namespace adchpp

#endif // LOGMANAGER_H
