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

#include "ManagedSocket.h"
#include "SocketManager.h"
#include <boost/asio/ip/address.hpp>

namespace adchpp
{
	using namespace std;

	using namespace boost::asio;

	ManagedSocket::ManagedSocket(SocketManager& sm, const AsyncStreamPtr& sock_, const ServerInfoPtr& aServer)
	: sock(sock_), overflow(time::not_a_date_time), disc(time::not_a_date_time),
	  lastWrite(time::not_a_date_time), sm(sm), server(aServer)
	{
	}

	ManagedSocket::~ManagedSocket() noexcept
	{
		dcdebug("ManagedSocket deleted\n");
	}

	static size_t sum(const BufferList& l)
	{
		size_t bytes = 0;
		for (BufferList::const_iterator i = l.begin(); i != l.end(); ++i)
			bytes += (*i)->size();

		return bytes;
	}

	size_t ManagedSocket::getQueuedBytes() const
	{
		return sum(outBuf);
	}

	void ManagedSocket::write(const BufferPtr& buf, bool lowPrio /* = false */) noexcept
	{
		if (buf->size() == 0 || disconnecting()) return;
		size_t queued = getQueuedBytes();
		if (sm.getMaxBufferSize() > 0 && queued + buf->size() > sm.getMaxBufferSize())
		{
			if (lowPrio)
				return;
			if (!overflow.is_not_a_date_time() && overflow + time::millisec(sm.getOverflowTimeout()) < time::now())
			{
				disconnect(REASON_WRITE_OVERFLOW);
				return;
			}
			overflow = time::now();
		}

		sm.getStats().queueBytes += buf->size();
		sm.getStats().queueCalls++;

		outBuf.push_back(buf);

		prepareWrite();
	}

	// Simplified handlers to avoid bind complexity
	namespace
	{

		/** Keeper keeps a reference to the managed socket */
		struct Keeper
		{
			Keeper(const ManagedSocketPtr& ms_) : ms(ms_)
			{
			}
			ManagedSocketPtr ms;

			void operator()(const boost::system::error_code& ec, size_t bytes)
			{
			}
		};

		template <void (ManagedSocket::*F)(const boost::system::error_code&, size_t)>
		struct Handler : Keeper
		{
			Handler(const ManagedSocketPtr& ms) : Keeper(ms)
			{
			}

			void operator()(const boost::system::error_code& ec, size_t bytes)
			{
				(ms.get()->*F)(ec, bytes);
			}
		};

		struct Disconnector
		{
			Disconnector(const AsyncStreamPtr& stream_) : stream(stream_)
			{
			}
			void operator()()
			{
				stream->close();
			}
			AsyncStreamPtr stream;
		};

	} // namespace

	void ManagedSocket::prepareWrite() noexcept
	{
		if (!writing()) // Not writing
		{
			if (!outBuf.empty())
			{
				lastWrite = time::now();
				sock->write(outBuf, Handler<&ManagedSocket::completeWrite>(shared_from_this()));
			}
		}
		else if (time::now() > lastWrite + time::seconds(60))
		{
			disconnect(REASON_WRITE_TIMEOUT);
		}
	}

	void ManagedSocket::completeWrite(const boost::system::error_code& ec, size_t bytes) noexcept
	{
		lastWrite = time::not_a_date_time;

		if (!ec)
		{
			sm.getStats().sendBytes += bytes;
			sm.getStats().sendCalls++;

			while (bytes > 0)
			{
				BufferPtr& p = *outBuf.begin();
				if (p->size() <= bytes)
				{
					bytes -= p->size();
					outBuf.erase(outBuf.begin());
				}
				else
				{
					p = make_shared<Buffer>(p->data() + bytes, p->size() - bytes);
					bytes = 0;
				}
			}

			if (!overflow.is_not_a_date_time())
			{
				size_t left = getQueuedBytes();
				if (left < sm.getMaxBufferSize())
				{
					overflow = time::not_a_date_time;
				}
			}

			if (disconnecting() && outBuf.empty())
				sock->shutdown(Keeper(shared_from_this()));
			else
				prepareWrite();
		}
		else
			fail(REASON_SOCKET_ERROR, ec.message());
	}

	void ManagedSocket::prepareRead() noexcept
	{
		// We first send in an empty buffer to get notification when there's data
		// available
		sock->prepareRead(BufferPtr(), Handler<&ManagedSocket::prepareRead2>(shared_from_this()));
	}

	void ManagedSocket::prepareRead2(const boost::system::error_code& ec, size_t) noexcept
	{
		if (!ec)
		{
			// ADC commands are typically small - using a small buffer
			// helps with fairness
			// Calling available() on an ASIO socket seems to be terribly slow
			// Also, we might end up here if the socket has been closed, in which
			// case available would return 0 bytes...
			// We can't make a synchronous receive here because when using SSL
			// there might be data on the socket that won't translate into user data
			// and thus read_some will block
			// If there's no user data, this will effectively post a read operation
			// with a buffer and waste memory...to be continued.
			inBuf = make_shared<Buffer>(64);
			sock->prepareRead(inBuf, Handler<&ManagedSocket::completeRead>(shared_from_this()));
		}
		else
			fail(REASON_SOCKET_ERROR, ec.message());
	}

	void ManagedSocket::completeRead(const boost::system::error_code& ec, size_t bytes) noexcept
	{
		if (!ec)
		{
			try
			{
				sm.getStats().recvBytes += bytes;
				sm.getStats().recvCalls++;

				inBuf->resize(bytes);

				if (dataHandler) dataHandler(inBuf);

				inBuf.reset();
				prepareRead();
			}
			catch (const boost::system::system_error& e)
			{
				fail(REASON_SOCKET_ERROR, e.code().message());
			}
		}
		else
		{
			inBuf.reset();
			fail(REASON_SOCKET_ERROR, ec.message());
		}
	}

	void ManagedSocket::completeAccept(const boost::system::error_code& ec) noexcept
	{
		if (!ec)
		{
			if (connectedHandler) connectedHandler();
			sock->init(std::bind(&ManagedSocket::ready, shared_from_this()));
		}
		else
			fail(REASON_SOCKET_ERROR, ec.message());
	}

	void ManagedSocket::ready() noexcept
	{
		if (readyHandler) readyHandler();
		prepareRead();
	}

	void ManagedSocket::fail(Reason reason, const std::string& info) noexcept
	{
		if (failedHandler)
		{
			failedHandler(reason, info);

			connectedHandler = nullptr;
			readyHandler = nullptr;
			dataHandler = nullptr;
			failedHandler = nullptr;
		}
	}

	struct Reporter
	{
		Reporter(ManagedSocketPtr ms,
				 void (ManagedSocket::*f)(Reason reason, const std::string& info),
				 Reason reason,
				 const std::string& info)
		: ms(ms), f(f), reason(reason), info(info)
		{
		}

		void operator()()
		{
			(ms.get()->*f)(reason, info);
		}

		ManagedSocketPtr ms;
		void (ManagedSocket::*f)(Reason reason, const std::string& info);

		Reason reason;
		std::string info;
	};

	bool ManagedSocket::getHbriParams(AdcCommand& cmd) const noexcept
	{
		if (!isV6())
		{
			if (!server->address6.empty())
				cmd.addParam("I6", server->address6);
			else
				return false;
			cmd.addParam("P6", server->port);
		}
		else
		{
			if (!server->address4.empty())
				cmd.addParam("I4", server->address4);
			else
				return false;
			cmd.addParam("P4", server->port);
		}
		return true;
	}

	bool ManagedSocket::isV6() const noexcept
	{
		using namespace boost::asio::ip;
		address remote;
		remote = address::from_string(ip);
		if (remote.is_v4() || (remote.is_v6() && remote.to_v6().is_v4_mapped())) return false;
		return true;
	}

	void ManagedSocket::disconnect(Reason reason, const std::string& info) noexcept
	{
		if (disconnecting()) return;
		const auto timeout = sm.getDisconnectTimeout();
		disc = time::now() + time::millisec(timeout);
		sm.addJob(Reporter(shared_from_this(), &ManagedSocket::fail, reason, info));
		if (!writing()) sock->shutdown(Keeper(shared_from_this()));
		sm.addJob(timeout, Disconnector(sock));
	}

	bool ManagedSocket::disconnecting() const
	{
		return !disc.is_not_a_date_time();
	}

	bool ManagedSocket::writing() const
	{
		return !lastWrite.is_not_a_date_time();
	}
} // namespace adchpp
