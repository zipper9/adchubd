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

#ifndef ADCHPP_ADCHPP_CORE_H_
#define ADCHPP_ADCHPP_CORE_H_

#include "Utils.h"
#include "forward.h"

namespace adchpp
{

	/** A single instance of an entire hub with plugins, settings and listening
	 * sockets */
	class Core
	{
	public:
		typedef std::function<void()> Callback;
		~Core();

		static std::shared_ptr<Core> create(const std::string& configPath);

		void run();

		void shutdown();

		LogManager& getLogManager();
		SocketManager& getSocketManager();
		PluginManager& getPluginManager();
		ClientManager& getClientManager();

		const std::string& getConfigPath() const { return configPath; }
		const std::string& getDataPath() const { return dataPath; }
		void setDataPath(const std::string& path) { dataPath = path; }

		/** execute a function asynchronously */
		void addJob(const Callback& callback) noexcept;

		/** execute a function after the specified amount of time
		 * @param msec milliseconds
		 */
		void addJob(const long msec, const Callback& callback);

		/** execute a function after the specified amount of time
		 * @param time a string that obeys to the "[-]h[h][:mm][:ss][.fff]" format
		 */
		void addJob(const std::string& time, const Callback& callback);

		/** execute a function at regular intervals
		 * @param msec milliseconds
		 * @return function one must call to cancel the timer (its callback will still
		 * be executed)
		 */
		Callback addTimedJob(const long msec, const Callback& callback);

		/** execute a function at regular intervals
		 * @param time a string that obeys to the "[-]h[h][:mm][:ss][.fff]" format
		 * @return function one must call to cancel the timer (its callback will still
		 * be executed)
		 */
		Callback addTimedJob(const std::string& time, const Callback& callback);

		time::ptime getStartTime() const { return startTime; }

	private:
		Core(const std::string& configPath);

		void init();

		void doShutdown(); /// @todo remove when we have lambdas

		std::unique_ptr<LogManager> lm;
		std::unique_ptr<SocketManager> sm;
		std::unique_ptr<PluginManager> pm;
		std::unique_ptr<ClientManager> cm;

		const std::string configPath;
		std::string dataPath;
		time::ptime startTime;
	};

} // namespace adchpp

#endif /* CORE_H_ */
