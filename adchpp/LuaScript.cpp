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

#include "LuaScript.h"
#include "LuaEngine.h"
#include "LogManager.h"
#include "AppPaths.h"

#ifdef _WIN32
#include <direct.h>
#else
#ifndef MAX_PATH
#define MAX_PATH PATH_MAX
#endif

#endif

extern "C"
{
#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>
}

using std::string;
using namespace adchpp;

const string LuaScript::className = "LuaScript";

LuaScript::LuaScript(Engine* engine) : Script(engine)
{
}

LuaScript::~LuaScript()
{
	getEngine()->call("unloaded", filename);
}

void LuaScript::loadFile(const string& path, const string& filename)
{
	this->filename = filename;

	char old_dir[MAX_PATH];
	if (!getcwd(old_dir, MAX_PATH))
		old_dir[0] = 0;

	auto absPath = AppPaths::makeAbsolutePath(path);
	if (chdir(absPath.c_str()) != 0)
	{
		// LOG(className, "Unable to change to directory " + absPath);
	}
	else
	{
		int error = luaL_loadfile(getEngine()->l, filename.c_str()) || lua_pcall(getEngine()->l, 0, 0, 0);

		if (error)
		{
			fprintf(stderr, "Error loading file: %s\n", lua_tostring(getEngine()->l, -1));
			// LOG(className, string("Error loading file: ") + lua_tostring(getEngine()->l, -1));
		}
		else
		{
			// LOG(className, "Loaded " + filename);
			getEngine()->call("loaded", filename);
		}

		if (old_dir[0])
		{
			chdir(old_dir);
		}
	}
}

void LuaScript::getStats(string& str) const
{
	str += filename + "\n";
}

LuaEngine* LuaScript::getEngine() const
{
	return static_cast<LuaEngine*>(engine);
}
