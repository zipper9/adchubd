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

#ifndef BASELIB_NOEXCEPT_H_
#define BASELIB_NOEXCEPT_H_

// for compilers that don't support noexcept, use an exception specifier

#ifdef __clang__

#elif defined(__GNUC__)

#if (__GNUC__ < 4 || (__GNUC__ == 4 && __GNUC_MINOR__ < 6)) && !defined(noexcept)
// GCC 4.6 is the first GCC to implement noexcept.
#define noexcept throw()
#endif

#elif defined(_MSC_VER)

#if _MSC_VER < 1900 && !defined(noexcept)
#define noexcept throw()
#endif

#endif

#endif // BASELIB_NOEXCEPT_H_
