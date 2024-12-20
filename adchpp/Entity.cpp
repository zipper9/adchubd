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

#include <baselib/StrUtil.h>
#include "Entity.h"
#include "ClientManager.h"
#include "Tag16.h"

namespace adchpp
{

	Entity::~Entity()
	{
		for (PluginDataMap::iterator i = pluginData.begin(), iend = pluginData.end(); i != iend; ++i)
			(*i->first)(i->second);
	}

	void Entity::inject(AdcCommand& cmd)
	{
		cm.onReceive(*this, cmd);
	}

	const std::string& Entity::getField(const char* name) const
	{
		auto i = fields.find(AdcCommand::toField(name));
		return i == fields.end() ? Util::emptyString : i->second;
	}

	bool Entity::hasField(const char* name) const
	{
		return fields.find(AdcCommand::toField(name)) != fields.end();
	}

	void Entity::setField(const char* name, const std::string& value)
	{
		uint16_t code = AdcCommand::toField(name);

		if (code == AdcCommand::toField("SU"))
		{
			filters.clear();
			if ((value.size() + 1) % 5 == 0)
			{
				filters.reserve((value.size() + 1) / 5);
				for (size_t i = 0; i < value.size(); i += 5)
				{
					filters.push_back(AdcCommand::toFourCC(value.data() + i));
				}
			}
		}

		if (value.empty())
			fields.erase(code);
		else
			fields[code] = value;

		INF = BufferPtr();
	}

	bool Entity::getAllFields(AdcCommand& cmd) const noexcept
	{
		for (auto i = fields.begin(); i != fields.end(); ++i)
			cmd.addParam(AdcCommand::fromField(i->first), i->second);
		return !fields.empty();
	}

	static bool isFieldSupported(uint16_t code)
	{
		return code != TAG('P', 'D');
	}

	void Entity::updateFields(const AdcCommand& cmd)
	{
		dcassert(cmd.getCommand() == AdcCommand::CMD_INF);
		for (auto j = cmd.getParameters().begin(); j != cmd.getParameters().end(); ++j)
		{
			if (j->length() < 2)
				continue;
			const char* c = j->c_str();
			if (isFieldSupported(AdcCommand::toField(c)))
				setField(c, j->substr(2));
		}
	}

	const BufferPtr& Entity::getINF() const
	{
		if (!INF)
		{
			AdcCommand cmd(AdcCommand::CMD_INF,
				getSID() == AdcCommand::HUB_SID ? AdcCommand::TYPE_INFO : AdcCommand::TYPE_BROADCAST,
				getSID());
			getAllFields(cmd);
			INF = cmd.getBuffer();
		}
		return INF;
	}

	bool Entity::addSupports(uint32_t feature)
	{
		if (std::find(supports.begin(), supports.end(), feature) != supports.end())
			return false;

		supports.push_back(feature);
		SUP = BufferPtr();
		return true;
	}

	StringList Entity::getSupportList() const
	{
		StringList ret(supports.size());
		for (size_t i = 0; i < supports.size(); ++i)
			ret[i] = AdcCommand::fromFourCC(supports[i]);
		return ret;
	}

	bool Entity::removeSupports(uint32_t feature)
	{
		std::vector<uint32_t>::iterator i = std::find(supports.begin(), supports.end(), feature);
		if (i == supports.end()) return false;
		supports.erase(i);
		SUP = BufferPtr();
		return true;
	}

	const BufferPtr& Entity::getSUP() const
	{
		if (!SUP)
		{
			AdcCommand cmd(AdcCommand::CMD_SUP,
				getSID() == AdcCommand::HUB_SID ? AdcCommand::TYPE_INFO : AdcCommand::TYPE_BROADCAST,
				getSID());
			for (std::vector<uint32_t>::const_iterator i = supports.begin(), iend = supports.end(); i != iend; ++i)
				cmd.addParam("AD", AdcCommand::fromFourCC(*i));
			SUP = cmd.getBuffer();
		}
		return SUP;
	}

	bool Entity::hasSupport(uint32_t feature) const
	{
		return find(supports.begin(), supports.end(), feature) != supports.end();
	}

	void Entity::updateSupports(const AdcCommand& cmd) noexcept
	{
		for (auto i = cmd.getParameters().begin(); i != cmd.getParameters().end(); ++i)
		{
			const std::string& str = *i;
			if (str.size() != 6) continue;
			if (str[0] == 'A' && str[1] == 'D')
				addSupports(AdcCommand::toFourCC(str.c_str() + 2));
			else if (str[0] == 'R' && str[1] == 'M')
				removeSupports(AdcCommand::toFourCC(str.c_str() + 2));
		}
	}

	bool Entity::hasClientSupport(uint32_t feature) const
	{
		return std::find(filters.begin(), filters.end(), feature) != filters.end();
	}

	bool Entity::removeClientSupport(uint32_t feature)
	{
		auto f = std::find(filters.begin(), filters.end(), feature);
		if (f == filters.end()) return false;
		filters.erase(f);
		auto& infSupports = fields.find(AdcCommand::toField("SU"))->second;
		auto p = infSupports.find(AdcCommand::fromFourCC(feature));
		dcassert(p != std::string::npos);
		infSupports.erase(p, 5);
		if (!infSupports.empty() && infSupports.back() == ',')
			infSupports.erase(supports.size() - 1);
		return true;
	}

	bool Entity::isFiltered(const std::string& features) const
	{
		if (filters.empty()) return true;

		for (size_t i = 0; i < features.size(); i += 5)
		{
			if (features[i] == '-')
			{
				if (std::find(filters.begin(), filters.end(), AdcCommand::toFourCC(features.data() + i + 1)) != filters.end())
					return true;
			}
			else if (features[i] == '+')
			{
				if (std::find(filters.begin(), filters.end(), AdcCommand::toFourCC(features.data() + i + 1)) == filters.end())
					return true;
			}
		}
		return false;
	}

	void Entity::setPluginData(const PluginDataHandle& handle, void* data) noexcept
	{
		clearPluginData(handle);
		pluginData.insert(std::make_pair(handle, data));
	}

	void* Entity::getPluginData(const PluginDataHandle& handle) const noexcept
	{
		PluginDataMap::const_iterator i = pluginData.find(handle);
		return i == pluginData.end() ? 0 : i->second;
	}

	void Entity::clearPluginData(const PluginDataHandle& handle) noexcept
	{
		PluginDataMap::iterator i = pluginData.find(handle);
		if (i == pluginData.end()) return;

		(*i->first)(i->second);
		pluginData.erase(i);
	}

	void Entity::setFlag(size_t flag)
	{
		flags.setFlag(flag);
		if (flag & MASK_CLIENT_TYPE)
			setField("CT", Util::toString(flags.getFlags() & MASK_CLIENT_TYPE));
	}

	void Entity::unsetFlag(size_t flag)
	{
		flags.unsetFlag(flag);
		if (flag & MASK_CLIENT_TYPE)
			setField("CT", Util::toString(flags.getFlags() & MASK_CLIENT_TYPE));
	}

	size_t Entity::getQueuedBytes() const
	{
		return 0;
	}

	time::ptime Entity::getOverflow() const
	{
		return time::not_a_date_time;
	}

} // namespace adchpp
