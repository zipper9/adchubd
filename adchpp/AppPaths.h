#ifndef APP_PATHS_H_
#define APP_PATHS_H_

#include <string>

namespace AppPaths
{
	std::string getModuleFileName();
	std::string makeAbsolutePath(const std::string& filename);
	std::string makeAbsolutePath(const std::string& path, const std::string& filename);
}

#endif // APP_PATHS_H_
