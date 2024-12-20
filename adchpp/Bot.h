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

#ifndef BOT_H_
#define BOT_H_

#include "forward.h"
#include "Entity.h"

namespace adchpp
{

	struct BotRemover;

	class Bot : public Entity
	{
	public:
		typedef std::function<void(Bot& bot, const BufferPtr& cmd)> SendHandler;

		Bot(ClientManager& cm, uint32_t sid, const SendHandler& handler_);

		virtual int getType() const { return TYPE_BOT; }
		virtual void send(const BufferPtr& cmd) { if (handler) handler(*this, cmd); }
		virtual void disconnect(Reason reason, const std::string& info) noexcept;

		using Entity::send;

	private:
		friend struct BotRemover;

		SendHandler handler;
		bool disconnecting;

		void die(Reason reason, const std::string& info);
	};

} // namespace adchpp

#endif /* BOT_H_ */
