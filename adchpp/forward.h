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

#ifndef ADCHPP_ADCHPP_FORWARD_H_
#define ADCHPP_ADCHPP_FORWARD_H_

#include <vector>
#include <memory>

namespace adchpp
{

	class Client;
	class ClientManager;
	class Core;
	class Entity;
	class LogManager;

	class ManagedSocket;
	typedef std::shared_ptr<ManagedSocket> ManagedSocketPtr;

	class PluginManager;

	struct ServerInfo;
	typedef std::shared_ptr<ServerInfo> ServerInfoPtr;
	typedef std::vector<ServerInfoPtr> ServerInfoList;

	class SocketFactory;
	typedef std::shared_ptr<SocketFactory> SocketFactoryPtr;

	class SocketManager;

} // namespace adchpp

#endif /*FORWARD_H_*/
