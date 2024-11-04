#ifndef BASELIB_STR_UTIL_H_
#define BASELIB_STR_UTIL_H_

#include <locale.h>
#include <stdlib.h>
#include <string>
#include <type_traits>

#ifdef _UNICODE
#define toStringT toStringW
#define toHexStringT toHexStringW
#else
#define toStringT toString
#define toHexStringT toHexString
#endif

namespace Util
{
	using std::string;
	using std::wstring;

	inline bool isWhiteSpace(int c)
	{
		return c == ' ' || c == '\f' || c == '\n' || c == '\r' || c == '\t' || c == '\v';
	}

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable:4146)
#endif

	template<typename int_type, typename char_type>
	inline int_type stringToInt(const char_type* s)
	{
		while (isWhiteSpace(*s)) s++;
		bool negative = false;
		if (std::is_signed<int_type>::value && *s == '-')
		{
			negative = true;
			s++;
		}
		else if (*s == '+')
			s++;
		int_type result = 0;
		while (*s >= '0' && *s <= '9')
		{
			result = result*10 + *s - '0';
			s++;
		}
		return negative ? -result : result;
	}

	template<typename int_type, typename char_type>
	inline int_type stringToInt(const char_type* s, size_t& index)
	{
		bool negative = false;
		if (std::is_signed<int_type>::value && s[index] == '-')
		{
			negative = true;
			index++;
		}
		else if (s[index] == '+')
			index++;
		int_type result = 0;
		while (s[index] >= '0' && s[index] <= '9')
		{
			result = result*10 + s[index] - '0';
			index++;
		}
		return negative ? -result : result;
	}

#ifdef _MSC_VER
#pragma warning(pop)
#endif

	inline int toInt(const string& s)  { return stringToInt<int, char>(s.c_str()); }
	inline int toInt(const char* s)    { return stringToInt<int, char>(s); }
	inline int toInt(const wstring& s) { return stringToInt<int, wchar_t>(s.c_str()); }
	inline int toInt(const wchar_t* s) { return stringToInt<int, wchar_t>(s); }

	inline uint32_t toUInt32(const string& s)  { return stringToInt<uint32_t, char>(s.c_str()); }
	inline uint32_t toUInt32(const char* s)    { return stringToInt<uint32_t, char>(s); }
	inline uint32_t toUInt32(const wstring& s) { return stringToInt<uint32_t, wchar_t>(s.c_str()); }
	inline uint32_t toUInt32(const wchar_t* s) { return stringToInt<uint32_t, wchar_t>(s); }

	inline int64_t toInt64(const string& s)  { return stringToInt<int64_t, char>(s.c_str()); }
	inline int64_t toInt64(const char* s)    { return stringToInt<int64_t, char>(s); }
	inline int64_t toInt64(const wstring& s) { return stringToInt<int64_t, wchar_t>(s.c_str()); }
	inline int64_t toInt64(const wchar_t* s) { return stringToInt<int64_t, wchar_t>(s); }

	inline bool toDouble(double& result, const string& s)
	{
		if (s.empty())
		{
			result = 0;
			return false;
		}
		// Work-around for atof and locales...
		char* endptr;
		lconv* lv = localeconv();
		string::size_type i = s.find_last_of(".,");
		if (i != string::npos && s[i] != lv->decimal_point[0])
		{
			string tmp(s);
			tmp[i] = lv->decimal_point[0];
			result = strtod(tmp.c_str(), &endptr);
			return *endptr == 0;
		}
		result = strtod(s.c_str(), &endptr);
		return *endptr == 0;
	}

	inline double toDouble(const string& s)
	{
		double result;
		toDouble(result, s);
		return result;
	}

	template<typename int_type, typename char_type>
	inline std::basic_string<char_type> intToString(int_type v)
	{
		static const size_t BUF_SIZE = 64;
		char_type s[BUF_SIZE];
		size_t i = BUF_SIZE;
		bool negative = false;
		if (std::is_signed<int_type>::value && v < 0)
			negative = true;
		while (v)
		{
			int r = (int) (v % 10);
			v /= 10;
			if (negative)
				s[--i] = (char_type) ('0' - r);
			else
				s[--i] = (char_type) ('0' + r);
		}
		if (i == BUF_SIZE) s[--i] = '0';
		if (negative) s[--i] = '-';
		return std::basic_string<char_type>(s + i, BUF_SIZE - i);
	}

	template<typename int_type, typename char_type>
	inline std::basic_string<char_type> uintToHexString(int_type v)
	{
		static_assert(std::is_unsigned<int_type>::value, "int_type must be unsigned");
		static const size_t BUF_SIZE = 16;
		char_type s[BUF_SIZE];
		size_t i = BUF_SIZE;
		while (v)
		{
			unsigned dig = (unsigned) (v & 0xF);
			s[--i] = (char_type) "0123456789ABCDEF"[dig];
			v >>= 4;
		}
		if (i == BUF_SIZE) s[--i] = '0';
		return std::basic_string<char_type>(s + i, BUF_SIZE - i);
	}

	inline string  toString(int val)  { return intToString<int, char>(val); }
	inline wstring toStringW(int val) { return intToString<int, wchar_t>(val); }

	inline string  toString(unsigned val)  { return intToString<unsigned, char>(val); }
	inline wstring toStringW(unsigned val) { return intToString<unsigned, wchar_t>(val); }

	inline string  toString(long val)  { return intToString<long, char>(val); }
	inline wstring toStringW(long val) { return intToString<long, wchar_t>(val); }

	inline string  toString(unsigned long val)  { return intToString<unsigned long, char>(val); }
	inline wstring toStringW(unsigned long val) { return intToString<unsigned long, wchar_t>(val); }

	inline string  toString(long long val)  { return intToString<long long, char>(val); }
	inline wstring toStringW(long long val) { return intToString<long long, wchar_t>(val); }

	inline string  toString(unsigned long long val)  { return intToString<unsigned long long, char>(val); }
	inline wstring toStringW(unsigned long long val) { return intToString<unsigned long long, wchar_t>(val); }

	inline string toString(double val)
	{
		static const int BUF_SIZE = 512;
		char buf[BUF_SIZE];
		snprintf(buf, BUF_SIZE, "%0.2f", val);
		return buf;
	}

	inline wstring toStringW(double val)
	{
		static const int BUF_SIZE = 512;
		wchar_t buf[BUF_SIZE];
#if defined(_MSC_VER) && _MSC_VER < 1900
		_snwprintf(buf, BUF_SIZE, L"%0.2f", val);
		buf[BUF_SIZE-1] = 0;
#else
		swprintf(buf, BUF_SIZE, L"%0.2f", val);
#endif
		return buf;
	}

	inline string  toHexString(unsigned long val)  { return uintToHexString<unsigned long, char>(val); }
	inline wstring toHexStringW(unsigned long val) { return uintToHexString<unsigned long, wchar_t>(val); }

	inline string  toHexString(const void* val)  { return uintToHexString<size_t, char>(reinterpret_cast<size_t>(val)); }
	inline wstring toHexStringW(const void* val) { return uintToHexString<size_t, wchar_t>(reinterpret_cast<size_t>(val)); }
}

#endif // BASELIB_STR_UTIL_H_
