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

#include "version.h"

#define VERSIONSTRING "1.0.0"
#define VERSIONFLOAT 1.0F

#ifndef NDEBUG
#define BUILDSTRING "Debug"
#else
#define BUILDSTRING "Release"
#endif

#define FULLVERSIONSTRING VERSIONSTRING " " BUILDSTRING

namespace adchpp
{

	std::string appName = APPNAME;
	std::string versionString = FULLVERSIONSTRING;
	float versionFloat = VERSIONFLOAT;

} // namespace adchpp
