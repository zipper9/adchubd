%module luadchpp
%include "lua_fnptr.i"

/*
in addition to the elements defined here and in adchpp.i, the Lua interface also includes:
- scriptPath: absolute path to the script directory (not necessarily the current dir).
- loaded(filename), unloaded(filename): functions called (if they exist in the global environment)
after a script has been loaded or unloaded.
- loading(filename), unloading(filename): functions called (if they exist in the global environment)
before a script is being loaded or unloaded. return true to discard further processing.
*/

%inline %{
/* Deleter for per-entity objects */
static void free_lua_ref(void* data) {
	SWIGLUA_REF* ref = reinterpret_cast<SWIGLUA_REF*>(data);
	swiglua_ref_clear(ref);
	delete ref;
}
%}

typedef unsigned int size_t;

%{
	static adchpp::Core *getCurrentCore(lua_State *l) {
		lua_getglobal(l, "currentCore");
		void *core = lua_touserdata(l, lua_gettop(l));
		lua_pop(l, 1);
		return reinterpret_cast<Core*>(core);
	}

	namespace adchpp {
		const std::string &getConfigPath(lua_State *l);
	}
%}

%wrapper %{

static int traceback (lua_State *L) {
	lua_getfield(L, LUA_GLOBALSINDEX, "debug");
	if (!lua_istable(L, -1)) {
		printf("No debug table\n");
		lua_pop(L, 1);
		return 1;
	}
	lua_getfield(L, -1, "traceback");
	if (!lua_isfunction(L, -1)) {
		printf("No traceback in debug\n");
		lua_pop(L, 2);
		return 1;
	}

	lua_pushvalue(L, 1); /* pass error message */
	lua_pushinteger(L, 2); /* skip this function and traceback */
	lua_call(L, 2, 1); /* call debug.traceback */
	return 1;
}

class RegistryItem : private boost::noncopyable {
public:
	RegistryItem(lua_State* L_) : L(L_), index(luaL_ref(L, LUA_REGISTRYINDEX)) {
	}
	~RegistryItem() {
		luaL_unref(L, LUA_REGISTRYINDEX, index);
	}

	void push() { lua_rawgeti(L, LUA_REGISTRYINDEX, index); }
private:
	lua_State* L;
	int index;
};

class LuaFunction {
public:
	LuaFunction(lua_State* L_) : L(L_), registryItem(new RegistryItem(L_)) { }

	void operator()() {
		pushFunction();
		docall(0, 0);
	}

	void operator()(adchpp::Entity& c) {
		pushFunction();

		SWIG_NewPointerObj(L, &c, SWIGTYPE_p_adchpp__Entity, 0);
		docall(1, 0);
	}

	void operator()(adchpp::Entity& c, const std::string& str) {
		pushFunction();

		SWIG_NewPointerObj(L, &c, SWIGTYPE_p_adchpp__Entity, 0);
		lua_pushstring(L, str.c_str());

		docall(2, 0);
	}

	void operator()(adchpp::Entity& c, int i) {
		pushFunction();

		SWIG_NewPointerObj(L, &c, SWIGTYPE_p_adchpp__Entity, 0);
		lua_pushinteger(L, i);

		docall(2, 0);
	}

	void operator()(adchpp::Entity& c, adchpp::AdcCommand& cmd) {
		pushFunction();

		SWIG_NewPointerObj(L, &c, SWIGTYPE_p_adchpp__Entity, 0);
		SWIG_NewPointerObj(L, &cmd, SWIGTYPE_p_adchpp__AdcCommand, 0);

		docall(2, 0);
	}

	void operator()(adchpp::Entity& c, adchpp::AdcCommand& cmd, bool& i) {
		pushFunction();

		SWIG_NewPointerObj(L, &c, SWIGTYPE_p_adchpp__Entity, 0);
		SWIG_NewPointerObj(L, &cmd, SWIGTYPE_p_adchpp__AdcCommand, 0);
		lua_pushboolean(L, i);

		if(docall(3, 1) != 0) {
			return;
		}

		if(lua_isboolean(L, -1)) {
			i &= lua_toboolean(L, -1) == 1;
		}
		lua_pop(L, 1);
	}
	
	void operator()(adchpp::Entity& c, const adchpp::AdcCommand& cmd, bool& i) {
		pushFunction();

		SWIG_NewPointerObj(L, &c, SWIGTYPE_p_adchpp__Entity, 0);
		SWIG_NewPointerObj(L, &cmd, SWIGTYPE_p_adchpp__AdcCommand, 0);
		lua_pushboolean(L, i);

		if(docall(3, 1) != 0) {
			return;
		}

		if(lua_isboolean(L, -1)) {
			i &= lua_toboolean(L, -1) == 1;
		}
		lua_pop(L, 1);
	}

	void operator()(const SimpleXML& s) {
		pushFunction();

		SWIG_NewPointerObj(L, &s, SWIGTYPE_p_SimpleXML, 0);
		docall(1, 0);
	}

	void operator()(adchpp::Entity& c, const StringList& cmd, bool& i) {
		pushFunction();

		SWIG_NewPointerObj(L, &c, SWIGTYPE_p_adchpp__Entity, 0);
		SWIG_NewPointerObj(L, &cmd, SWIGTYPE_p_std__vectorT_std__string_t, 0);
		lua_pushboolean(L, i);

		if(docall(3, 1) != 0) {
			return;
		}

        if(lua_isboolean(L, -1)) {
			i &= lua_toboolean(L, -1) == 1;
		}
        lua_pop(L, 1);
	}

	void operator()(adchpp::Entity& c, DCReason reason, const std::string& info) {
		pushFunction();

		SWIG_NewPointerObj(L, &c, SWIGTYPE_p_adchpp__Entity, 0);
		lua_pushnumber(L, reason);
		lua_pushstring(L, info.c_str());

		if(docall(3, 0) != 0) {
			return;
		}
	}

	void operator()(adchpp::Bot& bot, const adchpp::BufferPtr& buf) {
		pushFunction();

		SWIG_NewPointerObj(L, &bot, SWIGTYPE_p_adchpp__Bot, 0);
		SWIG_NewPointerObj(L, &buf, SWIGTYPE_p_shared_ptrT_adchpp__Buffer_t, 0);

		docall(2, 0);
	}

private:
	void pushFunction() {
		registryItem->push();
	}

	int docall(int narg, int nret) {
		int status;
		int base = lua_gettop(L) - narg;  /* function index */
		lua_pushcfunction(L, traceback);  /* push traceback function */
		lua_insert(L, base);  /* put it under chunk and args */
		status = lua_pcall(L, narg, nret, base);
		lua_remove(L, base);  /* remove traceback function */
		if(status == LUA_ERRRUN) {
			if (!lua_isnil(L, -1)) {
				const char *msg = lua_tostring(L, -1);
				if (msg == NULL) msg = "(error object is not a string)";
				fprintf(stderr, "%d, %d: %s\n", status, lua_type(L, -1), msg);
			} else {
				fprintf(stderr, "Lua error without error");
			}
			lua_pop(L, 1);
		} else if(status == LUA_ERRMEM) {
			fprintf(stderr, "Lua memory allocation error\n");
		} else if(status == LUA_ERRERR) {
			fprintf(stderr, "Lua error function error\n");
		} else if(status != 0) {
			fprintf(stderr, "Unknown lua status: %d\n", status);
		}

		return status;
	}

	lua_State* L;
	std::shared_ptr<RegistryItem> registryItem;
};

static int exec(lua_State* L) {
	void* p;
	if(SWIG_IsOK(SWIG_ConvertPtr(L, lua_upvalueindex(1), &p, SWIGTYPE_p_std__functionT_void_fF_t, 0))) {
		(*reinterpret_cast<std::function<void ()>*>(p))();
	}
	return 0;
}

%}

%typemap(in, checkfn="lua_isnumber") int64_t,uint64_t,const int64_t&, const uint64_t&,
// include unsigned in here to bypass the standard typemap for unsigned numbers that fails with HUB_SID
uint32_t, const uint32_t&
{
	$1 = ($1_ltype)lua_tonumber(L,$input);
}

%typemap(out) int64_t,uint64_t,const int64_t&, const uint64_t& {
   lua_pushnumber(L, (lua_Number)$1); SWIG_arg++;
}

%typemap(in) std::function<void () > {
	$1 = LuaFunction(L);
}

%typemap(out) std::function<void ()> {
	SWIG_NewPointerObj(L, new std::function<void ()>($1), SWIGTYPE_p_std__functionT_void_fF_t, 1);
	lua_pushcclosure(L, exec, 1);
	SWIG_arg++;
}

%typemap(in) std::function<void (adchpp::Entity &) > {
	$1 = LuaFunction(L);
}

%typemap(in) std::function<void (adchpp::Entity &, adchpp::AdcCommand &) > {
	$1 = LuaFunction(L);
}

%typemap(in) std::function<void (adchpp::Entity &, adchpp::AdcCommand &, bool&) > {
	$1 = LuaFunction(L);
}

%typemap(in) std::function<void (adchpp::Entity &, const adchpp::AdcCommand &, bool&) > {
	$1 = LuaFunction(L);
}

%typemap(in) std::function<void (adchpp::Entity &, int) > {
	$1 = LuaFunction(L);
}

%typemap(in) std::function<void (adchpp::Entity &, const std::string&) > {
	$1 = LuaFunction(L);
}

%typemap(in) std::function<void (const SimpleXML&) > {
	$1 = LuaFunction(L);
}

%typemap(in) std::function<void (adchpp::Entity &, const StringList&, bool&) > {
	$1 = LuaFunction(L);
}

%typemap(in) std::function<void (adchpp::Entity&, DCReason, const std::string&) > {
	$1 = LuaFunction(L);
}

%typemap(in) std::function<void (adchpp::Bot&, const adchpp::BufferPtr&) > {
	$1 = LuaFunction(L);
}

%include "embed.i"

%extend adchpp::AdcCommand {
	bool hasParam(const char* name, size_t start) {
		std::string tmp;
        return self->getParam(name, start, tmp);
	}

	std::string getParam(const char* name, size_t start) {
		std::string tmp;
		if(self->getParam(name, start, tmp)) {
			return tmp;
		}
		return std::string();
	}
}

%extend adchpp::Entity {
	SWIGLUA_REF getPluginData(const PluginDataHandle& handle) {
		void* ret = $self->getPluginData(handle);
		if(ret) {
			return *reinterpret_cast<SWIGLUA_REF*>(ret);
		}
		return SWIGLUA_REF();
	}

	void setPluginData(const PluginDataHandle& handle, SWIGLUA_REF data) {
		$self->setPluginData(handle, reinterpret_cast<void*>(new SWIGLUA_REF(data)));
	}

    void clearPluginData(const PluginDataHandle& handle) {
		$self->clearPluginData(handle);
	}
}

%extend adchpp::PluginManager {
	PluginDataHandle registerPluginData() {
		return self->registerPluginData(free_lua_ref);
	}
}

%extend std::map<std::string, int> {
	std::vector<std::string> keys() {
		std::vector<std::string> ret;
		
		for(std::map<std::string, int>::const_iterator i = $self->begin(), iend = $self->end(); i != iend; ++i) {
			ret.push_back(i->first);
		}
		
		return ret;
	}
}

%inline %{

namespace adchpp {
	ClientManager* getCM(lua_State* l) { return &getCurrentCore(l)->getClientManager(); }
	LogManager* getLM(lua_State* l) { return &getCurrentCore(l)->getLogManager(); }
	PluginManager* getPM(lua_State* l) { return &getCurrentCore(l)->getPluginManager(); }
	SocketManager* getSM(lua_State* l) { return &getCurrentCore(l)->getSocketManager(); }

	const std::string &getConfigPath(lua_State *l) { return getCurrentCore(l)->getConfigPath(); }
	const std::string &getDataPath(lua_State *l) { return getCurrentCore(l)->getDataPath(); }
	const std::string &Util_getCfgPath(lua_State *l) { return getConfigPath(l); }
	std::string Util_getLocalIp(lua_State *l) { return Utils::getLocalIp(); }
	std::string Util_formatBytes(lua_State *l, int64_t bytes) { return Util::formatBytes(bytes); }
}

%}
