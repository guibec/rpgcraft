
#include "x-types.h"
#include "x-stl.h"
#include "x-stdlib.h"
#include "x-string.h"

#include "x-assertion.h"

#include "ajek-script.h"
#include "ajek-script-internal.h"

extern "C" {
#	include "linit.c"
}

DECLARE_MODULE_NAME("lua-main");

struct AjekScriptSettings {
	struct {
		u32		lua_print_enabled		: 1;		// 0 to mute all lualib print() statements
	};

	u64	all64;

	void SetDefaultState() {
		all64					= 0;
		lua_print_enabled		= 1;
	}
};

union AjekScriptTraceSettings {
	struct {
		u32		warn_module_globals		: 1;		// warn when new globals are created at module scope 
		u32		warn_enclosed_globals   : 1;		// warn when new globals are created outside module scope, eg. within any function
		u32		error_enclosed_globals  : 1;		// error when new globals are created outside module scope, eg. within any function  (takes precedence over warn when enabled)
		u32		trace_gcmem				: 1;		// enables gcmem usage checks at every entry and exit point for ajek framework libs
	};

	u64		all64;

	void SetDefaultState() {
		all64					= 0;

		warn_module_globals		= 0;
		warn_enclosed_globals   = 1;
		error_enclosed_globals  = 1;
		trace_gcmem				= 0;
	}
};

AjekScriptSettings		g_ScriptConfig;
AjekScriptTraceSettings	g_ScriptTrace;

static bool		s_script_settings_initialized = 0;
static xString	s_script_dbg_path_prefix;

void AjekScriptEnv::PrintStackTrace()
{
	lua_Debug info1;

	int level = 1;
	while(lua_getstack(m_L, level, &info1))
	{
		lua_getinfo (m_L, "lS",		&info1);

		lua_Debug info0;
		lua_getstack(m_L, level-1,	&info0);
		lua_getinfo (m_L, "n",		&info0);

		log_host("    %s(%d) : %s()", info1.short_src, info1.currentline, info0.name ? info0.name : "(module)" );
		++level;
	}
}

void AjekScript_SetDebugAbsolutePaths(const xString& cwd, const xString& target)
{
	// Paths internally are relative to the CWD, but when we log errors to console we
	// want them to be relative to the dir specified here, which is typically the project 
	// or solution dir (Visual Studio).

	// Generating such relative paths isn't especially easy.  There's no built-in functions
	// for such logic in C++ or Windows.  Maybe in the future ...

	//  c:\aa\bb\cc\sln\file.sln
	//  c:\aa\bb\dd\proj\file.txt
	//
	// Common Path = c:\aa\bb\
	//
	// 1. Convert both paths into absolute forms.
	// 2. Remove common part from both (=UncommonSln, =UncommonTgt)
	//       [MSDN PathCommonPrefix(), tho straight up string check should be fine once PathCanonicalize() is used]
	// 3. replace all valid path parts with .. (=DotDots)
	// 4. Path.Join(DotDots, UncommonTarget);


	// For now: force paths to absolute:
	s_script_dbg_path_prefix = cwd;
}

void AjekScript_SetDebugRelativePath(const xString& relpath)
{
	// Paths internally are relative to the CWD, but when we log errors to console we
	// want them to be relative to the dir specified here, which is typically the project 
	// or solution dir (Visual Studio).

	s_script_dbg_path_prefix = relpath;
}

// luaL_error() is expected to be called from a C function (lualib), which means the parameter for "where" is 1,
// when we actually need it to be zero when in the context fot he Lua VM itself.

int luaVM_error (lua_State *L, const char *fmt, ...) {
  va_list argp;
  va_start(argp, fmt);
  luaL_where(L, 0);
  lua_pushvfstring(L, fmt, argp);
  va_end(argp);
  lua_concat(L, 2);
  return lua_error(L);
}

extern "C" void ajek_warn_new_global(lua_State* L)
{
	lua_Debug info0;
	lua_getstack(L, 0,		&info0);
	lua_getinfo (L, "nlS",	&info0);

	// Setting globals from module scope is OK unless full global trace is enabled.
	if (!info0.name && !g_ScriptTrace.warn_module_globals) {
		return;
	}

	if (info0.name && g_ScriptTrace.error_enclosed_globals) {
		luaVM_error(L, "new global variable created outside module scope.\n");
	}

	if (info0.name && !g_ScriptTrace.warn_enclosed_globals) {
		return;
	}

	warn_host("\n%s(%d): WARN: new global variable created", info0.short_src, info0.currentline);

	// log function name in future?
	//info0.name ? info0.name : "[module]", info0.name ? "()" : "" );
}

/* number of chars of a literal string without the ending \0 */
#define LL(x)			(sizeof(x)/sizeof(char) - 1)
#define addstr(a,b,l)	( memcpy(a,b,(l) * sizeof(char)), a += (l) )
#define RETS			"..."

extern "C" void ajek_lua_ChunkId_Filename(char* out, const char* source, size_t bufflen)
{
	xString result = xPath_Combine(s_script_dbg_path_prefix, source);

	size_t l = result.GetLength();
    if (l <= bufflen)  /* small enough? */
		memcpy(out, result.c_str(), l * sizeof(char) + 1);
    else {  /* add '...' before rest of name */
		addstr(out, RETS, LL(RETS));
		bufflen -= LL(RETS);
		memcpy(out, result.c_str() + l - bufflen, bufflen * sizeof(char) + 1);
    }
}

extern "C" void ajek_lua_printf(const char *fmt, ...)
{
	va_list list;
	va_start(list, fmt);
	if (g_ScriptConfig.lua_print_enabled) {
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

void AjekScript_InitSettings()
{
	s_script_settings_initialized = 1;
	g_ScriptConfig.SetDefaultState	();
	g_ScriptTrace.SetDefaultState	();
}

void AjekScript_InitModuleList()
{
	bug_on(!s_script_settings_initialized);

	g_script_env[ScriptEnv_AppConfig]	.Alloc(); 
	g_script_env[ScriptEnv_Game]		.Alloc(); 
}

void AjekScriptEnv::Alloc()
{
	// Future MSPACE registration goes here.
}

void AjekScriptEnv::NewState()
{
	DisposeState();

	m_L = luaL_newstate();
	log_and_abort_on( !m_L, "Create new Lua state failed." );
	log_host( "lua_stack = %s", cPtrStr(m_L) );
	luaL_openlibs(m_L);
}

void AjekScriptEnv::DisposeState()
{
	if (!m_L)		return;

	// Wipes entire Lua state --- should delete and re-create MSPACE as well.
	log_host( "Disposing AjekScript Environment...");
	lua_close(m_L);

	m_L			= nullptr;
	m_has_error = false;
}

void AjekScriptEnv::PrintLastError() const
{
	bug_on(!m_has_error);		// if not set then the upvalue's not going to be a lua error string ...
	xPrintLn( xFmtStr("\n%s", lua_tostring(m_L, -1)));
}


void AjekScriptEnv::LoadModule(const xString& path)
{
	m_has_error = false;

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

	//assume(s_module_id == LuaModule_State_NotLoading);
	//s_module_id = module_id;

	// parse and execute!
	//   # Add '@' to front of string to inform LUA that it's a filename.
	//     (lua will even automatically use ... to abbreviate long paths!)

	if (ret != LUA_ERRSYNTAX) {
		//if (trace_libajek) {
		//	log_tooling("[FnTrace] ExecModule(\"%s\")", filename);
		//	s_fntrace_nesting += '>';
		//}

		ret = lua_pcall(m_L, 0, 0, 0);

		//if (trace_libajek) {
		//	s_fntrace_nesting.PopBack();
		//}
	}

	// Cleanup!
	//  # Free memory
	//  # remove init-only APIs.

	//s_module_id	= LuaModule_State_NotLoading;
	
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

		m_has_error = true;
		PrintLastError();
	}

	GCUSAGE(m_L);
	// grammar checks
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

	lua_getglobal(m_L, varname.c_str());
	result.m_isNil = lua_isnil(m_L, -1);
	result.m_value = lua_tostring(m_L, -1);

	if (!result.m_value) {
		bug_on(!result.m_isNil);
		result.m_value = "";
	}
	lua_pop(m_L, 1);
	return result;
}

LuaTableScope::~LuaTableScope() throw()
{
	if (m_env) {
		m_env->m_open_global_table.Clear();
	}
	m_env = nullptr;
}

// Returns FALSE if varname is either nil or not a table.  It is considered the responsibility of the
// caller to use glob_IsNil() check ahead of this call if they want to implement special handling of
// nil separate from non-table-type behavior.
LuaTableScope::LuaTableScope(AjekScriptEnv& env, const char* tableName)
{
	m_env = &env;

	// TODO : Maybe this can be modified to automatically close out previous lua_getglobal stack... ?
	bug_on_qa(!m_env->m_open_global_table.IsEmpty(), "Another global table is already opened.");

	lua_getglobal(m_env->m_L, tableName);

	m_isNil		=				lua_isnil  (m_env->m_L, -1);
	m_isTable	= !m_isNil &&	lua_istable(m_env->m_L, -1);

	if (m_isTable) {
		m_env->m_open_global_table = tableName;
	}
}

lua_string LuaTableScope::get_string(const xString& key)
{
	auto* L = m_env->m_L;
    lua_pushstring(L, key);
    lua_gettable(L, -2);

	lua_string	result;

	// lua_tostring() modifies the table by converting integers to strings.
	// This may not be desirable in some situations.  Think about it!

	result.m_isNil = lua_isnil(L, -1);
	result.m_value = lua_tostring(L, -1);

	if (!result.m_value) {
		bug_on(!result.m_isNil);
		result.m_value = "";
	}

	lua_pop(L, 1);
	return result;
}


//	env.table_get_string("ModulePath");

// Retrieve results from lua_pcall
//      /* retrieve result */
//      if (!lua_isnumber(L, -1))
//        error(L, "function `f' must return a number");
//      z = lua_tonumber(L, -1);
//      lua_pop(L, 1);  /* pop returned value */

// Goals:
//  1. create module for engine parameter setup.   Debug settings, running environment, paths, so on.
//  2. create module for specifying behavior of tile engine things.
//       tilesets to load.
//       starting position.
//       response to user input.