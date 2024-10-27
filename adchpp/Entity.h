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

#ifndef ADCHPP_ENTITY_H
#define ADCHPP_ENTITY_H

#include "AdcCommand.h"
#include "Buffer.h"
#include "CID.h"
#include "Plugin.h"
#include "TimeUtil.h"
#include "forward.h"

namespace adchpp
{

	class Entity : private boost::noncopyable
	{
	public:
		enum State
		{
			/** Initial protocol negotiation (wait for SUP) */
			STATE_PROTOCOL,
			/** Validating the secondary address */
			STATE_HBRI,
			/** Identify the connecting client (wait for INF) */
			STATE_IDENTIFY,
			/** Verify the client (wait for PAS) */
			STATE_VERIFY,
			/** Normal operation */
			STATE_NORMAL,
			/** Binary data transfer */
			STATE_DATA
		};

		enum Flag
		{
			FLAG_BOT = 0x01,
			FLAG_REGISTERED = 0x02,
			FLAG_OP = 0x04,
			FLAG_SU = 0x08,
			FLAG_OWNER = 0x10,
			FLAG_HUB = 0x20,
			FLAG_HIDDEN = 0x40,
			MASK_CLIENT_TYPE = FLAG_BOT | FLAG_REGISTERED | FLAG_OP | FLAG_SU | FLAG_OWNER | FLAG_HUB | FLAG_HIDDEN,

			FLAG_PASSWORD = 0x100,

			/** Extended away, no need to send msg */
			FLAG_EXT_AWAY = 0x200,

			/** Plugins can use these flags to disable various checks */
			/** Bypass ip check */
			FLAG_OK_IP = 0x400,

			/** This entity is now a ghost being disconnected, totally ignored by ADCH++
			 */
			FLAG_GHOST = 0x800,

			/** Set in the login phase if the users provides an IP for another protocol
			 **/
			FLAG_VALIDATE_HBRI = 0x1000
		};

		Entity(ClientManager& cm, uint32_t sid_) : sid(sid_), state(STATE_PROTOCOL), cm(cm)
		{
		}

		void send(const AdcCommand& cmd)
		{
			send(cmd.getBuffer());
		}
		virtual void send(const BufferPtr& cmd) = 0;

		virtual void inject(AdcCommand& cmd);

		const std::string& getField(const char* name) const;
		bool hasField(const char* name) const;
		void setField(const char* name, const std::string& value);

		/** Add any flags that have been updated to the AdcCommand (type etc is not
		 * set) */
		bool getAllFields(AdcCommand& cmd) const noexcept;
		const BufferPtr& getINF() const;

		bool addSupports(uint32_t feature);
		StringList getSupportList() const;
		bool hasSupport(uint32_t feature) const;
		bool removeSupports(uint32_t feature);

		bool hasClientSupport(uint32_t feature) const;
		bool removeClientSupport(uint32_t feature);

		/** Remove supports for the protocol that wasn't used for connecting **/
		void stripProtocolSupports() noexcept;

		const BufferPtr& getSUP() const;

		uint32_t getSID() const
		{
			return sid;
		}

		bool isFiltered(const std::string& features) const;

		void updateFields(const AdcCommand& cmd);
		void updateSupports(const AdcCommand& cmd) noexcept;

		/**
		 * Set PSD (plugin specific data). This allows a plugin to store arbitrary
		 * per-client data, and retrieve it later on. The life cycle of the data
		 * follows that of the client unless explicitly removed. Any data referenced
		 * by the plugin will have its delete function called when the Entity is
		 * deleted.
		 * @param id Id as retrieved from PluginManager::getPluginId()
		 * @param data Data to store, this can be pretty much anything
		 */
		void setPluginData(const PluginDataHandle& handle, void* data) noexcept;

		/**
		 * @param handle Plugin data handle, as returned by
		 * PluginManager::registerPluginData
		 * @return Value stored, NULL if none found
		 */
		void* getPluginData(const PluginDataHandle& handle) const noexcept;

		/**
		 * Clear any data referenced by the handle, calling the registered delete
		 * function.
		 */
		void clearPluginData(const PluginDataHandle& handle) noexcept;

		const CID& getCID() const { return cid; }
		void setCID(const CID& cid_) { cid = cid_; }

		State getState() const { return state; }
		void setState(State state_) { state = state_; }

		bool isSet(size_t flag) const { return flags.isSet(flag); }
		bool isAnySet(size_t flag) const { return flags.isAnySet(flag); }
		void setFlag(size_t flag);
		void unsetFlag(size_t flag);

		virtual void disconnect(Util::Reason reason, const std::string& info = Util::emptyString) = 0;

		/** The number of bytes in the write buffer */
		virtual size_t getQueuedBytes() const;

		/** The time that this entity's write buffer size exceeded the maximum buffer
		 * size, 0 if no overflow */
		virtual time::ptime getOverflow() const;

	protected:
		virtual ~Entity();

		typedef std::map<PluginDataHandle, void*> PluginDataMap;

		CID cid;
		uint32_t sid;
		Flags flags;
		State state;

		/** SUP items */
		std::vector<uint32_t> supports;

		/** INF SU */
		std::vector<uint32_t> filters;

		/** INF fields */
		std::map<uint16_t, std::string> fields;

		/** Plugin data, see PluginManager::registerPluginData */
		PluginDataMap pluginData;

		/** Latest INF cached */
		mutable BufferPtr INF;

		/** Latest SUP cached */
		mutable BufferPtr SUP;

		/** ClientManager that owns this entity */
		ClientManager& cm;
	};

} // namespace adchpp

#endif /* ADCHPP_ENTITY_H */
