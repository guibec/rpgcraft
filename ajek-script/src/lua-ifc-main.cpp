
#include "x-types.h"
#include "x-stl.h"
#include "x-stdlib.h"
#include "x-string.h"

#include "x-assertion.h"

#include "ajek-script.h"
#include "lua-ifc-internal.h"

extern "C" {
#	include "linit.c"
}

DECLARE_MODULE_NAME("lua-main");

bool g_script_log_verbose	= 1;

extern "C" void ajek_lua_printf(const char *fmt, ...)
{
	va_list list;
	va_start(list, fmt);
	if (g_script_log_verbose) {
		_host_log_v(xLogFlag_Default, "Lua", fmt, list);
		if (!TARGET_CONSOLE) {
			// to support 'tail' utils via CLI
			// limited to non-console builds to avoid potential perf impacts, and on orbis/xbox
			// use of the orbis console stream should be preferred.
			flush_log();
		}
	}
	va_end(list);
}

extern "C" void ajek_lua_abort()
{
	log_and_abort( "lua aborted!" );

	// abort is technically unreachable -- just here for __noreturn adherence, since log_abort()
	// is not always marked as noreturn depending on the build target configuration.
	if (!xIsDebuggerAttached()) {
		abort();
	}
}

//
// DynSpline's Game-Embedded Dir Structure:
//   * Lua include files should be loaded relative to the CWD.
//   * Absolute paths are supported in development environments only, and are
//     specified at runtime via game engine application config.
//   * CPATH is considered unused.  All C++ bindings should be provided by the
//     game engine binary.  CPATH is therefore unspecified.

//   Register set LUA_NOENV ...   lua_setfield(L, LUA_REGISTRYINDEX, tname);  /* registry.name = metatable */

// TODO - Long term plan here:
//   * Implement custom heap for lua environment (using dlmalloc, mspace, Windows HeapAlloc, etc)

static int ajek_test(lua_State* L)
{
	log_host ("Test Pased!");
	return 0;
}

static const luaL_Reg ajeklib[] = {
	{"testfunc",     ajek_test},
	{NULL, NULL}
};

LUAMOD_API int luaopen_ajek (lua_State *L) {
  luaL_newlib(L, ajeklib);
  return 1;
}

AjekScriptEnv	g_script_env[NUM_SCRIPT_ENVIRONMENTS];

void AjekScript_Alloc()
{
	// generate mspaces here, in the future.
}

void AjekScript_InitModuleList()
{
	g_script_env[ScriptEnv_AppConfig].Alloc(); 

	g_script_env[ScriptEnv_Game]		.Alloc(); 
	g_script_env[ScriptEnv_Game]		.RegisterFrameworkLibs(); 
}

void AjekScriptEnv::Alloc()
{
	m_L = luaL_newstate();
	log_and_abort_on( !m_L, "Create new Lua state failed." );
	log_host( "lua_stack = %s", cPtrStr(m_L) );
	luaL_openlibs(m_L);
}

void AjekScriptEnv::LoadModule(const xString& path)
{
	bug_on (!m_L,  "Invalid object state: uninitialized script environment.");
	bug_on (path.IsEmpty());
	if (path.IsEmpty()) return;

	//ScopedFpRoundMode	nearest;

	log_host( "LoadModule: %s", path.c_str() );

	// TODO: register init-only global C functions here ...
	// [...]
	// END

	int ret;
	ret = luaL_loadfile(m_L, path);
	if (ret && ret != LUA_ERRSYNTAX) {
		log_and_abort( "luaL_loadfile failed : %s", lua_tostring(m_L, -1) );
	}

	//assume(luaBridge::s_module_id == LuaModule_State_NotLoading);
	//luaBridge::s_module_id = module_id;

	// parse and execute!
	//   # Add '@' to front of string to inform LUA that it's a filename.
	//     (lua will even automatically use ... to abbreviate long paths!)

	if (ret != LUA_ERRSYNTAX) {
		//if (luaBridge::g_config.trace_hooks || luaBridge::g_config.trace_functions) {
		//	log_tooling("[FnTrace] ExecModule(\"%s\")", filename);
		//	luaBridge::s_fntrace_nesting += '>';
		//}

		ret = lua_pcall(m_L, 0, 0, 0);

		//if (luaBridge::g_config.trace_hooks || luaBridge::g_config.trace_functions) {
		//	luaBridge::s_fntrace_nesting.PopBack();
		//}
	}

	// Cleanup!
	//  # Free memory
	//  # remove init-only APIs.

	//luaBridge::s_module_id	= LuaModule_State_NotLoading;
	
	// Error Handling!
	//  # ScriptDebug fall into a recoverable SyntaxError state, from which the user can
	//    modify scripts and then resume execution.
	//  # All other build targets abort on error here.
	//

	if (!AJEK_SCRIPT_DEBUGGER) {
		log_and_abort_on(ret, "script load error.\n%s", lua_tostring(m_L, -1) );
	}

	if (ret) {
		// Visual studio looks for src relative to the *.sln directory when resolving error
		// codes.  The filenames returned by Lua are relative to the CWD of the instance.

		log_host( "\n%s", lua_tostring(m_L, -1) );
		log_host( "\ndyn-spline\\%s", lua_tostring(m_L, -1) );
		//luaBridge::s_script_syntax_error = true;
	}

	GCUSAGE(m_L);
	// grammar checks
//	luaBridge_GrammarCheck();

}

void AjekScriptEnv::RegisterFrameworkLibs()
{
	luaL_requiref(m_L, "ajek", luaopen_ajek, 1);
    lua_pop(m_L, 1);  /* remove lib */
}

lua_State* AjekScriptEnv::getLuaState()
{
	bug_on(!m_L);
	return m_L;
}

const lua_State* AjekScriptEnv::getLuaState() const
{
	bug_on(!m_L);
	return m_L;
}

AjekScriptEnv& AjekScriptEnv_Get(ScriptEnvironId moduleId)
{
	bug_on(moduleId >= bulkof(g_script_env));
	return g_script_env[moduleId];
}


lua_State* AjekScript_GetLuaState(ScriptEnvironId moduleId)
{
	bug_on(moduleId >= bulkof(g_script_env));
	return g_script_env[moduleId].getLuaState();
}

bool AjekScriptEnv::glob_IsNil(const xString& varname) const
{
	lua_getglobal(m_L, varname.c_str());
	bool result = !!lua_isnil(m_L, -1);
	lua_pop(m_L, 1);
	return result;

}

lua_s32	AjekScriptEnv::glob_get_s32(const xString& varname) const
{
	lua_s32	result;
	lua_getglobal(m_L, varname.c_str());
	result.m_isNil = lua_isnil(m_L, -1);

	s64 intval = luaL_checkinteger(m_L, -1);
	result.m_value = _NCS32(intval);

	if(!IsInInt32(intval)) {
		warn_host( "glob_get_s32(\"%s\") - Integer truncation %s -> %s",
			varname, cHexStr(intval), cHexStr(result.m_value));
	}

	lua_pop(m_L, 1);
	return result;
}

lua_bool AjekScriptEnv::glob_get_bool(const xString& varname) const
{
	lua_bool result;
	lua_getglobal(m_L, varname.c_str());
	result.m_isNil = lua_isnil(m_L, -1);

	bug_on_qa( !lua_isboolean(m_L, -1), "%s isn't a boolean variable, it's %s.", varname.c_str(), lua_typename(m_L, lua_type(m_L, -1)) );
	result.m_value = lua_toboolean(m_L, -1);
	lua_pop(m_L, 1);
	return result;
}

lua_string AjekScriptEnv::glob_get_string(const xString& varname) const
{
	lua_string	result;
	size_t		out_len;
	lua_getglobal(m_L, varname.c_str());
	result.m_isNil = lua_isnil(m_L, -1);
	result.m_value = lua_tolstring(m_L, -1, &out_len);

	if (!result.m_value) {
		bug_on(!result.m_isNil);
		result.m_value = "";
	}
	lua_pop(m_L, 1);
	return result;
}


// Goals:
//  1. create module for engine parameter setup.   Debug settings, running environment, paths, so on.
//  2. create module for specifying behavior of tile engine things.
//       tilesets to load.
//       starting position.
//       response to user input.