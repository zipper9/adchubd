#ifndef APP_PATHS_H_
#define APP_PATHS_H_

#include <baselib/tstring.h>

namespace AppPaths
{
	tstring getModuleFileName();
	tstring getModuleDirectory();
	string makeAbsolutePath(const string& filename);
	string makeAbsolutePath(const string& path, const string& filename);
#ifdef _UNICODE
	wstring makeAbsolutePath(const wstring& filename);
	wstring makeAbsolutePath(const wstring& path, const wstring& filename);
#endif
}

#endif // APP_PATHS_H_
