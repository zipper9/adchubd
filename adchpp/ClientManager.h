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

#ifndef ADCHPP_CLIENTMANAGER_H
#define ADCHPP_CLIENTMANAGER_H

#include <baselib/TigerHash.h>
#include "AdcCommand.h"
#include "Bot.h"
#include "CID.h"
#include "Client.h"
#include "Hub.h"
#include "Signal.h"

namespace adchpp
{
	/**
	 * The ClientManager takes care of all protocol details, clients and so on. This
	 * is the very heart of ADCH++ together with SocketManager and ManagedSocket.
	 */
	class ClientManager : public CommandHandler<ClientManager>
	{
	public:
		typedef std::unordered_map<uint32_t, Entity*> EntityMap;
		typedef EntityMap::iterator EntityIter;

		/** @return SID of entity or AdcCommand::INVALID_SID if not found */
		uint32_t getSID(const std::string& nick) const noexcept;
		/** @return SID of entity or AdcCommand::INVALID_SID if not found */
		uint32_t getSID(const CID& cid) const noexcept;

		/** @return The entity associated with a certain SID, NULL if not found */
		Entity* getEntity(uint32_t aSid) noexcept;

		/** @return A new Bot instance in STATE_IDENTIFY; set CID, nick etc and call
		 * regBot */
		Bot* createBot(const Bot::SendHandler& handler);
		void regBot(Bot& bot);

		/**
		 * Get a list of all currently connected clients. (Don't change it, it's
		 * non-const so that you'll be able to get non-const clients out of it...)!!!)
		 */
		EntityMap& getEntities() noexcept
		{
			return entities;
		}

		/** Send a command to according to its type */
		void send(const AdcCommand& cmd) noexcept;

		/** Send a buffer to all connected entities */
		void sendToAll(const BufferPtr& buffer) noexcept;

		/** Send buffer to a single client regardless of type */
		void sendTo(const BufferPtr& buffer, uint32_t to);

		/**
		 * Enter IDENTIFY state.
		 * Call this if you stop the SUP command when in PROTOCOL state.
		 *
		 * @param sendData Send ISUP & IINF.
		 */
		void enterIdentify(Entity& c, bool sendData) noexcept;

		/**
		 * Enter VERIFY state. Call this if you stop
		 * an INF in the IDENTIFY state and want to check a password.
		 *
		 * @param sendData Send GPA.
		 * @return The random data that was sent to the client (if sendData was true,
		 * undefined otherwise).
		 */
		ByteVector enterVerify(Entity& c, bool sendData) noexcept;

		/**
		 * Enter NORMAL state. Call this if you stop an INF of a password-less
		 * client in IDENTIFY state or a PAS in VERIFY state.
		 *
		 * @param sendData Send all data as mandated by the protocol, including list
		 * of connected clients.
		 * @param sendOwnInf Set to true to broadcast the client's inf (i e when a
		 * plugin asks for password)
		 * @return false if the client was disconnected
		 */
		bool enterNormal(Entity& c, bool sendData, bool sendOwnInf) noexcept;

		/**
		 * Do all SUP verifications and update client data. Call if you stop SUP but
		 * still want the default processing.
		 */
		bool verifySUP(Entity& c, AdcCommand& cmd) noexcept;

		/**
		 * Do all INF verifications and update client data. Call if you stop INF but
		 * still want the default processing.
		 */
		bool verifyINF(Entity& c, AdcCommand& cmd) noexcept;

		/**
		 * Verify password
		 */
		bool verifyPassword(Entity& c, const std::string& password, const ByteVector& salt, const std::string& suppliedHash);
		bool verifyPassword(Entity& c, const std::string& password, const ByteVector& salt, const std::string& suppliedHash, TigerHash& tiger);

		/**
		 * Verify that there aren't too many sockets overflowing (indicates lack of
		 * bandwidth)
		 */
		bool verifyOverflow(Entity& c);

		/** Update the state of c (this fires signalState as well) */
		void setState(Entity& c, Entity::State newState) noexcept;

		size_t getQueuedBytes() noexcept;

		typedef SignalTraits<void(Entity&)> SignalConnected;
		typedef SignalTraits<void(Entity&)> SignalReady;
		typedef SignalTraits<void(Entity&, AdcCommand&, bool&)> SignalReceive;
		typedef SignalTraits<void(Entity&, const std::string&)> SignalBadLine;
		typedef SignalTraits<void(Entity&, const AdcCommand&, bool&)> SignalSend;
		typedef SignalTraits<void(Entity&, int)> SignalState;
		typedef SignalTraits<void(Entity&, Reason, const std::string&)> SignalDisconnected;

		/** A client has just connected. */
		SignalConnected::Signal& signalConnected()
		{
			return signalConnected_;
		}
		/** A client is now ready for read / write operations (TLS handshake
		 * completed). */
		SignalConnected::Signal& signalReady()
		{
			return signalReady_;
		}
		SignalReceive::Signal& signalReceive()
		{
			return signalReceive_;
		}
		SignalBadLine::Signal& signalBadLine()
		{
			return signalBadLine_;
		}
		SignalSend::Signal& signalSend()
		{
			return signalSend_;
		}
		SignalState::Signal& signalState()
		{
			return signalState_;
		}
		SignalDisconnected::Signal& signalDisconnected()
		{
			return signalDisconnected_;
		}

		void setMaxCommandSize(size_t newSize)
		{
			maxCommandSize = newSize;
		}
		size_t getMaxCommandSize() const
		{
			return maxCommandSize;
		}

		void setHbriTimeout(size_t millis)
		{
			hbriTimeout = millis;
		}
		size_t getHbriTimeout() const
		{
			return hbriTimeout;
		}
		void setLogTimeout(size_t millis)
		{
			logTimeout = millis;
		}
		size_t getLogTimeout() const
		{
			return logTimeout;
		}

		Core& getCore() const
		{
			return core;
		}
		void prepareSupports(bool addHbri);

	private:
		friend class Core;
		friend class Client;
		friend class Entity;
		friend class Bot;

		Core& core;

		std::list<std::pair<Client*, time::ptime>> logins;

		typedef std::unordered_map<std::string, std::pair<Client*, time::ptime>> TokenMap;
		TokenMap hbriTokens;

		EntityMap entities;
		typedef std::unordered_map<std::string, Entity*> NickMap;
		NickMap nicks;
		typedef std::unordered_map<CID, Entity*> CIDMap;
		CIDMap cids;

		Hub hub;

		size_t maxCommandSize;
		size_t logTimeout;
		size_t hbriTimeout;

		static const std::string className;

		friend class CommandHandler<ClientManager>;

		uint32_t makeSID();

		bool verifyNick(Entity& c, const std::string& nick) noexcept;
		bool verifyCID(Entity& c, const AdcCommand& cmd, CID& cid) noexcept;
		bool verifyIp(Client& c, AdcCommand& cmd, bool& validateSecondaryProtocol) noexcept;

		bool sendHBRI(Client& c);
		void maybeSend(Entity& c, const AdcCommand& cmd);

		void removeLogins(Entity& c) noexcept;
		void removeEntity(Entity& c, Reason reason, const std::string& info) noexcept;

		bool handle(AdcCommand::SUP, Entity& c, AdcCommand& cmd) noexcept;
		bool handle(AdcCommand::INF, Entity& c, AdcCommand& cmd) noexcept;
		bool handle(AdcCommand::TCP, Entity& c, AdcCommand& cmd) noexcept;
		bool handleDefault(Entity& c, AdcCommand& cmd) noexcept;

		template <typename T> bool handle(T, Entity& c, AdcCommand& cmd) noexcept
		{
			return handleDefault(c, cmd);
		}

		void handleIncoming(const ManagedSocketPtr& sock) noexcept;

		void onConnected(Client&) noexcept;
		void onReady(Client&) noexcept;
		void onReceive(Entity&, AdcCommand&) noexcept;
		void onBadLine(Client&, const std::string&) noexcept;
		void onFailed(Client&, Reason reason, const std::string& info) noexcept;

		void badState(Entity& c, const AdcCommand& cmd) noexcept;
		/** send a fatal STA, a QUI with TL-1, then disconnect. */
		void disconnect(Entity& c,
			Reason reason,
			const std::string& info,
			AdcCommand::Error error = AdcCommand::ERROR_PROTOCOL_GENERIC,
			const std::string& staParam = Util::emptyString,
			int aReconnectTime = -1);

		SignalConnected::Signal signalConnected_;
		SignalReady::Signal signalReady_;
		SignalReceive::Signal signalReceive_;
		SignalBadLine::Signal signalBadLine_;
		SignalSend::Signal signalSend_;
		SignalState::Signal signalState_;
		SignalDisconnected::Signal signalDisconnected_;

		ClientManager(Core& core) noexcept;
		void onTimerSecond();

		void failHBRI(Client& cc);
		void stripProtocolSupports(Client& cc);
	};

} // namespace adchpp

#endif // CLIENTMANAGER_H
