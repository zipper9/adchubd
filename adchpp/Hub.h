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

#ifndef HUB_H_
#define HUB_H_

#include "Entity.h"

namespace adchpp
{

	class Hub : public Entity
	{
	public:
		Hub(ClientManager& cm);

		virtual int getType() const { return TYPE_HUB; }
		virtual void send(const BufferPtr& cmd) {}
		virtual void disconnect(Reason reason, const std::string&) noexcept {}
	};

} // namespace adchpp

#endif /* HUB_H_ */
