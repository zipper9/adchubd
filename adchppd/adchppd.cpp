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

#include <adchpp/Core.h>
#include <adchpp/ClientManager.h>
#include <adchpp/LogManager.h>
#include <adchpp/PluginManager.h>
#include <adchpp/SocketManager.h>
#include <adchpp/AppPaths.h>
#include <baselib/File.h>
#include <baselib/SimpleXML.h>
#include <baselib/PathUtil.h>

using namespace adchpp;

void loadXML(Core& core, const string& fileName)
{
	printf("Loading settings from %s\n", fileName.c_str());
	try
	{
		SimpleXML xml;
		xml.fromXML(File(fileName, File::READ, File::OPEN).read());
		xml.resetCurrentChild();
		xml.stepIn();
		while (xml.getNextChild())
		{
			if (xml.getChildTag() == "Settings")
			{
				xml.stepIn();
				while (xml.getNextChild())
				{
					const string& tag = xml.getChildTag();
					if (tag == "HubName")
					{
						core.getClientManager().getEntity(AdcCommand::HUB_SID)->setField("NI", xml.getChildData());
					}
					else if (tag == "Description")
					{
						core.getClientManager().getEntity(AdcCommand::HUB_SID)->setField("DE", xml.getChildData());
					}
					else if (tag == "Log")
					{
						core.getLogManager().setEnabled(xml.getChildData() == "1");
					}
					else if (tag == "LogFile")
					{
						core.getLogManager().setLogFile(xml.getChildData());
					}
					else if (tag == "DataPath")
					{
						string path = xml.getChildData();
						Util::toNativePathSeparators(path);
						Util::appendPathSeparator(path);
						core.setDataPath(path);
					}
					else if (tag == "MaxCommandSize")
					{
						core.getClientManager().setMaxCommandSize(Util::toInt(xml.getChildData()));
					}
					else if (tag == "BufferSize")
					{
						core.getSocketManager().setBufferSize(Util::toInt(xml.getChildData()));
					}
					else if (tag == "MaxBufferSize")
					{
						core.getSocketManager().setMaxBufferSize(Util::toInt(xml.getChildData()));
					}
					else if (tag == "OverflowTimeout")
					{
						core.getSocketManager().setOverflowTimeout(Util::toInt(xml.getChildData()));
					}
					else if (tag == "DisconnectTimeout")
					{
						core.getSocketManager().setDisconnectTimeout(Util::toInt(xml.getChildData()));
					}
					else if (tag == "LogTimeout")
					{
						core.getClientManager().setLogTimeout(Util::toInt(xml.getChildData()));
					}
					else if (tag == "HbriTimeout")
					{
						core.getClientManager().setHbriTimeout(Util::toInt(xml.getChildData()));
					}
				}
				xml.stepOut();
			}
			else if (xml.getChildTag() == "Servers")
			{
				xml.stepIn();
				ServerInfoList servers;
				while (xml.findChild("Server"))
				{
					ServerInfoPtr server = std::make_shared<ServerInfo>();
					server->port = xml.getChildAttrib("Port", Util::emptyString);

					server->bind4 = xml.getChildAttrib("BindAddress4", Util::emptyString);
					server->bind6 = xml.getChildAttrib("BindAddress6", Util::emptyString);
					server->address4 = xml.getChildAttrib("HubAddress4", Util::emptyString);
					server->address6 = xml.getChildAttrib("HubAddress6", Util::emptyString);

					if (xml.getBoolChildAttrib("TLS"))
					{
						server->TLSParams.cert = AppPaths::makeAbsolutePath(xml.getChildAttrib("Certificate"));
						server->TLSParams.pkey = AppPaths::makeAbsolutePath(xml.getChildAttrib("PrivateKey"));
						server->TLSParams.trustedPath = AppPaths::makeAbsolutePath(xml.getChildAttrib("TrustedPath"));
						server->TLSParams.dh = AppPaths::makeAbsolutePath(xml.getChildAttrib("DHParams"));
					}

#ifndef HAVE_OPENSSL
					if (server->secure())
						fprintf(stderr,
							"Error listening on port %s: This version hasn't been compiled with support for secure connections\n",
							server->port.c_str());
					else
#endif
						servers.push_back(server);
				}
				core.getSocketManager().setServers(servers);
				xml.stepOut();
			}
			else if (xml.getChildTag() == "Plugins")
			{
				core.getPluginManager().setPluginPath(xml.getChildAttrib("Path"));
				xml.stepIn();
				StringList plugins;
				while (xml.findChild("Plugin"))
				{
					plugins.push_back(xml.getChildData());
				}

				core.getPluginManager().setPluginList(plugins);
				xml.stepOut();
			}
		}

		xml.stepOut();
	}
	catch (const Exception& e)
	{
		printf("Unable to load configuration, using defaults: %s\n", e.getError().c_str());
	}
}
