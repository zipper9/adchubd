/*
 * Copyright (C) 2001-2017 Jacek Sieka, arnetheduck on gmail point com
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

#ifndef BASELIB_HASH_VALUE_H_
#define BASELIB_HASH_VALUE_H_

#include "TigerHash.h"
#include "Base32.h"
#include "BaseUtil.h"
#include <boost/functional/hash.hpp>

template<class Hasher>
struct HashValue
{
	static const size_t BITS = Hasher::BITS;
	static const size_t BYTES = Hasher::BYTES;

	HashValue()
	{
		memset(&data, 0, sizeof(data));
	}
	explicit HashValue(const uint8_t* src)
	{
		memcpy(data, src, BYTES);
	}
	explicit HashValue(const char* base32, unsigned len)
	{
		Util::fromBase32(base32, data, BYTES);
	}
	explicit HashValue(const std::string& base32)
	{
		Util::fromBase32(base32.c_str(), data, BYTES);
	}
	bool operator!=(const HashValue& rhs) const
	{
		return !(*this == rhs);
	}
	bool operator==(const HashValue& rhs) const
	{
		return memcmp(data, rhs.data, BYTES) == 0;
	}
	bool operator<(const HashValue& rhs) const
	{
		return memcmp(data, rhs.data, BYTES) < 0;
	}
	std::string toBase32() const
	{
		return Util::toBase32(data, BYTES);
	}
	std::string& toBase32(std::string& tmp) const
	{
		return Util::toBase32(data, BYTES, tmp);
	}
	size_t toHash() const
	{
		// RVO should handle this as efficiently as reinterpret_cast version
		size_t hvHash;
		memcpy(&hvHash, data, sizeof(hvHash));
		return hvHash;
	}
	bool isZero() const
	{
		for (size_t i = 0; i < BYTES; i++)
			if (data[i]) return false;
		return true;
	}

	uint8_t data[BYTES];
};

class TigerHash;
template<class Hasher> struct HashValue;
typedef HashValue<TigerHash> TTHValue;

namespace boost
{
template<typename T>
struct hash<HashValue<T>>
{
	size_t operator()(const HashValue<T>& rhs) const
	{
		return rhs.toHash();
	}
};
}

namespace std
{
template<typename T>
struct hash<HashValue<T> >
{
	size_t operator()(const HashValue<T>& rhs) const
	{
		return rhs.toHash();
	}
};

template<typename T>
struct hash<HashValue<T>* >
{
	size_t operator()(const HashValue<T>* rhs) const
	{
		return *(size_t*)rhs;
	}
};

template<typename T>
struct equal_to<HashValue<T>*>
{
	bool operator()(const HashValue<T>* lhs, const HashValue<T>* rhs) const
	{
		return (*lhs) == (*rhs);
	}
};

}

template<>
inline int compare(const TTHValue& v1, const TTHValue& v2)
{
	return memcmp(v1.data, v2.data, TTHValue::BYTES);
}

#endif // BASELIB_HASH_VALUE_H_
