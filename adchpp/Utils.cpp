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

#include "Utils.h"
#include <baselib/BaseUtil.h>
#include <string.h>

#ifdef _WIN32
#include <ws2tcpip.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#endif

#ifdef NDEBUG
#include "FastAlloc.h"
#endif

using std::string;

namespace adchpp
{
#ifdef NDEBUG
	FastCriticalSection FastAllocBase::mtx;
#endif

	string Utils::getLocalIp()
	{
		string tmp;

		char buf[256];
		gethostname(buf, 255);
		hostent* he = gethostbyname(buf);
		if (he == NULL || he->h_addr_list[0] == 0) return ::Util::emptyString;
		sockaddr_in dest;
		int i = 0;

		// We take the first ip as default, but if we can find a better one, use it
		// instead...
		memcpy(&dest.sin_addr, he->h_addr_list[i++], he->h_length);
		tmp = inet_ntoa(dest.sin_addr);
		if (strncmp(tmp.c_str(), "192", 3) == 0 || strncmp(tmp.c_str(), "169", 3) == 0 ||
		    strncmp(tmp.c_str(), "127", 3) == 0 || strncmp(tmp.c_str(), "10", 2) == 0)
		{
			while (he->h_addr_list[i])
			{
				memcpy(&(dest.sin_addr), he->h_addr_list[i], he->h_length);
				string tmp2 = inet_ntoa(dest.sin_addr);
				if (strncmp(tmp2.c_str(), "192", 3) != 0 && strncmp(tmp2.c_str(), "169", 3) != 0 &&
				    strncmp(tmp2.c_str(), "127", 3) != 0 && strncmp(tmp2.c_str(), "10", 2) != 0)
				{

					tmp = tmp2;
				}
				i++;
			}
		}
		return tmp;
	}

	bool Utils::isPrivateIp(string const& ip, bool v6)
	{
		if (v6) return strncmp(ip.c_str(), "fe80", 4) == 0;

		struct in_addr addr;
		addr.s_addr = inet_addr(ip.c_str());
		if (addr.s_addr != INADDR_NONE)
		{
			unsigned long haddr = ntohl(addr.s_addr);
			return ((haddr & 0xff000000) == 0x0a000000 || // 10.0.0.0/8
					(haddr & 0xff000000) == 0x7f000000 || // 127.0.0.0/8
					(haddr & 0xfff00000) == 0xac100000 || // 172.16.0.0/12
					(haddr & 0xffff0000) == 0xc0a80000);  // 192.168.0.0/16
		}
		return false;
	}

	bool Utils::validateCharset(string const& field, int p)
	{
		for (string::size_type i = 0; i < field.length(); ++i)
			if ((uint8_t) field[i] < p)
				return false;
		return true;
	}

}
