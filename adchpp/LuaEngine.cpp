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

#include "LuaEngine.h"
#include "LuaScript.h"
#include "Core.h"
#include "PluginManager.h"
#include "AppPaths.h"
#include <baselib/StrUtil.h>

extern "C"
{
#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>
}

using std::string;
using std::vector;
using namespace adchpp;

static void prepare_cpath(lua_State* L, const string& path)
{
	lua_getfield(L, LUA_GLOBALSINDEX, "package");
	if (!lua_istable(L, -1))
	{
		lua_pop(L, 1);
		return;
	}
	lua_getfield(L, -1, "cpath");
	if (!lua_isstring(L, -1))
	{
		lua_pop(L, 2);
		return;
	}

	string oldpath = lua_tostring(L, -1);
	oldpath += ";" + path + "?.so";
	lua_pushstring(L, oldpath.c_str());
	lua_setfield(L, -3, "cpath");

	// Pop table
	lua_pop(L, 2);
}

static void setScriptPath(lua_State* L, const string& path)
{
	lua_pushstring(L, path.c_str());
	lua_setglobal(L, "scriptPath");
}

LuaEngine::LuaEngine(Core& core) : core(core), luadchpp(false)
{
	l = lua_open();
	luaL_openlibs(l);
	lua_pushlightuserdata(l, &core);
	lua_setglobal(l, "currentCore");
	prepare_cpath(l, core.getPluginManager().getPluginPath());

	setScriptPath(l, Util::emptyString);
}

LuaEngine::~LuaEngine()
{
	std::vector<LuaScript*>::reverse_iterator it;
	while ((it = scripts.rbegin()) != scripts.rend())
		unloadScript(*it, true);

	if (l) lua_close(l);
}

/// @todo lambda
void LuaEngine::loadScript_(const string& path, const string& filename, LuaScript* script)
{
	script->loadFile(path, filename);
	scripts.push_back(script);
}

extern "C" void luaopen_luadchpp(lua_State* l);

Script* LuaEngine::loadScript(const string& path, const string& filename, const ParameterMap&)
{
	if (!luadchpp)
	{
		static const luaL_reg funcs = { nullptr, nullptr };
		luaopen_luadchpp(l);
		luaL_register(l, "luadchpp", &funcs);
		luadchpp = true;
	}
	setScriptPath(l, AppPaths::makeAbsolutePath(path));

	if (call("loading", filename)) return 0;

	LuaScript* script = new LuaScript(this);
	core.getPluginManager().attention(std::bind(&LuaEngine::loadScript_, this, path, filename, script));
	return script;
}

void LuaEngine::unloadScript(Script* s, bool force)
{
	if (call("unloading", static_cast<LuaScript*>(s)->filename) && !force) return;
	scripts.erase(remove(scripts.begin(), scripts.end(), s), scripts.end());
	delete s;
}

void LuaEngine::getStats(string& str) const
{
	str += "Lua engine\n";
	str += "\tUsed memory: " + Util::toString(lua_gc(l, LUA_GCCOUNT, 0)) + " KiB\n";
	str += "The following Lua scripts are loaded:\n";
	for (vector<LuaScript*>::const_iterator i = scripts.begin(); i != scripts.end(); ++i)
	{
		str += "\t";
		(*i)->getStats(str);
	}
}

bool LuaEngine::call(const string& f, const string& arg)
{
	lua_getfield(l, LUA_GLOBALSINDEX, f.c_str());
	if (!lua_isfunction(l, -1))
	{
		lua_pop(l, 1);
		return false;
	}

	lua_pushstring(l, arg.c_str());

	if (lua_pcall(l, 1, 1, 0) != 0)
	{
		lua_pop(l, 1);
		return false;
	}

	bool ret = lua_toboolean(l, -1);
	lua_pop(l, 1);
	return ret;
}
