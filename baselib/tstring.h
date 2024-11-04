#ifndef BASELIB_TSTRING_H_
#define BASELIB_TSTRING_H_

#include <string>

using std::string;
using std::wstring;

#ifdef _UNICODE
typedef wstring tstring;
typedef wchar_t tchar_t;
#else
typedef string tstring;
typedef char tchar_t;
#endif

#endif // BASELIB_TSTRING_H_
