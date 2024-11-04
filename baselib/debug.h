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

#ifndef BASELIB_DEBUG_H_
#define BASELIB_DEBUG_H_

#ifdef _DEBUG

#include <assert.h>
#include <stdarg.h>
#include <stdio.h>

#ifdef _WIN32
#include "w.h"
void DumpDebugMessage(const TCHAR *filename, const char *msg, size_t msgSize, bool appendNL);
#endif

static inline void debugTrace(const char* format, ...)
{
	va_list args;
	va_start(args, format);
#ifdef _WIN32
	char buf[512];
	int result = vsnprintf(buf, sizeof(buf), format, args);
	if (result > 0 && (size_t) result < sizeof(buf))
	{
		OutputDebugStringA(buf);
		DumpDebugMessage(_T("debug-trace.log"), buf, result, false);
	}
#else // _WIN32
	vprintf(format, args);
#endif // _WIN32
	va_end(args);
}

#define dcdebug debugTrace

#ifdef _MSC_VER
#include <crtdbg.h>
#define dcassert(exp) \
	do { \
		if (!(exp)) \
		{ \
			dcdebug("Assertion hit in %s(%d): " #exp "\n", __FILE__, __LINE__); \
			if (_CrtDbgReport(_CRT_ASSERT, __FILE__, __LINE__, NULL, #exp) == 1) \
				_CrtDbgBreak(); \
		} \
	} while (0)
#else
#define dcassert(exp) assert(exp)
#endif
#define dcdrun(exp) exp
#else //_DEBUG
#define dcdebug if (false) printf
#define dcassert(exp)
#define dcdrun(exp)
#endif //_DEBUG

#endif // BASELIB_DEBUG_H_
