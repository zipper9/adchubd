#ifndef BASELIB_BASE_UTIL_H_
#define BASELIB_BASE_UTIL_H_

#include "typedefs.h"

#ifdef _WIN32
#include "w.h"
#else
#include <errno.h>
#endif

template<typename T, bool flag> struct ReferenceSelector
{
	typedef T ResultType;
};
template<typename T> struct ReferenceSelector<T, true>
{
	typedef const T& ResultType;
};

template<typename T> class IsOfClassType
{
	public:
		template<typename U> static char check(int U::*);
		template<typename U> static float check(...);
	public:
		enum { Result = sizeof(check<T>(0)) };
};

template<typename T> struct TypeTraits
{
	typedef IsOfClassType<T> ClassType;
	typedef ReferenceSelector < T, ((ClassType::Result == 1) || (sizeof(T) > sizeof(char*))) > Selector;
	typedef typename Selector::ResultType ParameterType;
};

#define GETSET(type, name, name2) \
	private: type name; \
	public: TypeTraits<type>::ParameterType get##name2() const { return name; } \
	void set##name2(TypeTraits<type>::ParameterType a##name2) { name = a##name2; }

#define GETM(type, name, name2) \
	private: type name; \
	public: TypeTraits<type>::ParameterType get##name2() const { return name; }

#define GETC(type, name, name2) \
	private: const type name; \
	public: TypeTraits<type>::ParameterType get##name2() const { return name; }

/**
 * Compares two values
 * @return -1 if v1 < v2, 0 if v1 == v2 and 1 if v1 > v2
 */
template<typename T1>
inline int compare(const T1& v1, const T1& v2)
{
	return (v1 < v2) ? -1 : ((v1 == v2) ? 0 : 1);
}

namespace Util
{
	extern const tstring emptyStringT;
	extern const string emptyString;
	extern const wstring emptyStringW;
	extern const std::vector<uint8_t> emptyByteVector;

	string translateError(unsigned error) noexcept;
	inline string translateError() noexcept
	{
#ifdef _WIN32
		return translateError(GetLastError());
#else
		return translateError(errno);
#endif
	}
}

#endif // BASELIB_BASE_UTIL_H_
