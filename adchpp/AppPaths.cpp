#include "AppPaths.h"
#include <baselib/File.h>
#include <baselib/Text.h>
#include <baselib/PathUtil.h>

#ifdef _WIN32
#include <baselib/w.h>
#elif defined __APPLE__
#include <mach-o/dyld.h>
#elif defined __FreeBSD__
#include <sys/sysctl.h>
#else
#include <unistd.h>
#endif

#ifdef _WIN32
tstring AppPaths::getModuleFileName()
{
	static tstring moduleFileName;
	if (moduleFileName.empty())
	{
		WCHAR buf[MAX_PATH];
		DWORD len = GetModuleFileNameW(NULL, buf, MAX_PATH);
#ifdef _UNICODE
		moduleFileName.assign(buf, len);
#else
		Text::wideToUtf8(buf, len, moduleFileName);
#endif
	}
	return moduleFileName;
}
#else
string AppPaths::getModuleFileName()
{
	string result;
#ifdef __APPLE__
	char path[PATH_MAX];
	uint32_t size = sizeof(path);
	if (_NSGetExecutablePath(path, &size) == 0) result = path;
#elif defined __FreeBSD__
	int mib[4] = { CTL_KERN, KERN_PROC, KERN_PROC_PATHNAME, -1 };
	char buf[PATH_MAX];
	size_t cb = sizeof(buf);
	if (!sysctl(mib, 4, buf, &cb, NULL, 0)) result = buf;
#else
	char link[64];
	sprintf(link, "/proc/%u/exe", (unsigned) getpid());
	char buf[PATH_MAX];
	ssize_t size = readlink(link, buf, sizeof(buf));
	if (size > 0) result.assign(buf, size);
#endif
	return result;
}
#endif

tstring AppPaths::getModuleDirectory()
{
	tstring path = getModuleFileName();
	tstring::size_type pos = path.rfind(PATH_SEPARATOR);
	if (pos == tstring::npos)
		path.clear();
	else
		path.erase(pos + 1);
	return path;
}

string AppPaths::makeAbsolutePath(const string& filename)
{
	return makeAbsolutePath(Text::fromT(getModuleDirectory()), filename);
}

string AppPaths::makeAbsolutePath(const string& path, const string& filename)
{
	if (filename.empty() || File::isAbsolute(filename)) return filename;
	string result = path;
	Util::appendPathSeparator(result);
	result += filename;
	return result;
}

#ifdef _UNICODE
wstring AppPaths::makeAbsolutePath(const wstring& filename)
{
	return makeAbsolutePath(getModuleDirectory(), filename);
}

wstring AppPaths::makeAbsolutePath(const wstring& path, const wstring& filename)
{
	if (filename.empty() || File::isAbsolute(filename)) return filename;
	wstring result = path;
	Util::appendPathSeparator(result);
	result += filename;
	return result;
}
#endif
