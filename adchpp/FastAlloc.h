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

#ifndef ADCHPP_FASTALLOC_H
#define ADCHPP_FASTALLOC_H

#include <baselib/Locks.h>

namespace adchpp
{

#ifdef NDEBUG
	struct FastAllocBase
	{
		static FastCriticalSection mtx;
	};

	/**
	 * Fast new/delete replacements for constant sized objects, that also give nice
	 * reference locality...
	 */
	template <class T> struct FastAlloc : public FastAllocBase
	{
		// Custom new & delete that (hopefully) use the node allocator
		static void* operator new(size_t s)
		{
			if (s != sizeof(T)) return ::operator new(s);
			return allocate();
		}

		// Avoid hiding placement new that's needed by the stl containers...
		static void* operator new(size_t, void* m)
		{
			return m;
		}
		// ...and the warning about missing placement delete...
		static void operator delete(void*, void*)
		{
			// ? We didn't allocate so...
		}

		static void operator delete(void* m, size_t s)
		{
			if (s != sizeof(T))
				::operator delete(m);
			else if (m)
				deallocate((uint8_t*) m);
		}

	private:
		static void* allocate()
		{
			LockBase<FastCriticalSection> l(mtx);
			if (!freeList) grow();
			void* tmp = freeList;
			freeList = *((void**)freeList);
			return tmp;
		}

		static void deallocate(void* p)
		{
			LockBase<FastCriticalSection> l(mtx);
			*(void**) p = freeList;
			freeList = p;
		}

		static void* freeList;

		static void grow()
		{
			dcassert(sizeof(T) >= sizeof(void*));
			// We want to grow by approximately 128kb at a time...
			size_t items = (128 * 1024 + sizeof(T) - 1) / sizeof(T);
			freeList = new uint8_t[sizeof(T) * items];
			uint8_t* tmp = (uint8_t*)freeList;
			for (size_t i = 0; i < items - 1; i++)
			{
				*(void**)tmp = tmp + sizeof(T);
				tmp += sizeof(T);
			}
			*(void**) tmp = nullptr;
		}
	};
	template <class T> void* FastAlloc<T>::freeList = 0;
#else
	template <class T> struct FastAlloc
	{
	};
#endif

} // namespace adchpp

#endif // FASTALLOC_H
