
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
	union {
		struct {
			u32		lua_print_enabled		: 1;		// 0 to mute all lualib print() statements
		};

		u64	flags64;
	};

	xString		path_to_modules;

	void SetDefaultState() {
		flags64					= 0;
		lua_print_enabled		= 1;
	}
};

struct AjekScriptTraceSettings {
	struct {
		u32		warn_module_globals		: 1;		// warn when new globals are created at module scope
		u32		warn_enclosed_globals   : 1;		// warn when new globals are created outside module scope, eg. within any function
		u32		error_enclosed_globals  : 1;		// error when new globals are created outside module scope, eg. within any function  (takes precedence over warn when enabled)
		u32		trace_gcmem				: 1;		// enables gcmem usage checks at every entry and exit point for ajek framework libs
	};

	u64		flags64;

	void SetDefaultState() {
		flags64					= 0;

		warn_module_globals		= 0;
		warn_enclosed_globals   = 1;
		error_enclosed_globals  = 1;
		trace_gcmem				= 0;
	}
};

AjekScriptSettings		g_ScriptConfig;
AjekScriptTraceSettings	g_ScriptTrace;
AjekScriptEnv			g_script_env[NUM_SCRIPT_ENVIRONMENTS];

static bool		s_script_settings_initialized = 0;
static xString	s_script_dbg_path_prefix;

bool AjekScript_LoadConfiguration(AjekScriptEnv& env)
{
	if (auto& scriptConfig = env.glob_open_table("ScriptConfig")) {
		g_ScriptConfig.path_to_modules   = scriptConfig.get_string("ModulePath");
		g_ScriptConfig.lua_print_enabled = scriptConfig.get_bool("LuaPrintEnable");

		log_host_loud("   > ModulePath = %s", g_ScriptConfig.path_to_modules.c_str());
		if (!g_ScriptConfig.lua_print_enabled) {
			log_host_loud("   > Lua Print has been turned OFF!");
		}
	}

	return true;
}

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

void AjekScript_PrintBreakReloadMsg()
{
	xPrintLn("");
	xPrintLn("Debugger detected - Correct the problem and hit 'Continue' in debugger to reload and re-execute.");
}

void AjekScript_PrintDebugReloadMsg()
{
	xPrintLn("");
	xPrintLn("Execution paused due to error - Correct the problem and hit 'R' to reload and re-execute.");
}

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

	m_error	= AsError_None;
	m_L		= luaL_newstate();
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
	m_error		= AsError_None;		// important to clear this: error info is on the lua stack
	m_L			= nullptr;
}


void AjekScriptEnv::PrintLastError() const
{
	if (HasError()) {		// if not set then the upvalue's not going to be a lua error string ...
		xPrintLn( xFmtStr("\n%s", lua_tostring(m_L, -1)));
	}
}

AjekScriptError cvtLuaErrorToAjekError(int lua_error)
{
	switch (lua_error) {
		case LUA_YIELD		:	bug("LUA_YIELD.  HOW??");						break;
		case LUA_ERRRUN		:	return AsError_Runtime;							break;
		case LUA_ERRSYNTAX	:	return AsError_Syntax;							break;
		case LUA_ERRMEM		:	bug_qa("Lua Out Of Memory Error!");				break;
		case LUA_ERRGCMM	:	bug_qa("Lua Garbage Collector Failure!");		break;
		case LUA_ERRERR		:	return AsError_Assertion;						break;
		case LUA_ERRFILE	:   return AsError_Syntax;							break;
		default				:	bug_qa("Lua unknown error code = %d", lua_error);
	}

	return AsError_Environment;
}

void AjekScriptEnv::LoadModule(const xString& path)
{
	bug_on (!m_L,						"Invalid object state: uninitialized script environment.");
	bug_on (m_error != AsError_None,	"Invalid object state: script error is pending.");
	bug_on (path.IsEmpty());

	if (m_error != AsError_None)	return;
	if (path.IsEmpty())				return;

	//ScopedFpRoundMode	nearest;

	log_host( "LoadModule: %s", path.c_str() );

	// TODO: register init-only global C functions here ...
	// [...]
	// END

	int ret;
	ret = luaL_loadfile(m_L, path);
	bool isAbortErr = ret != LUA_ERRSYNTAX && ret != LUA_ERRFILE;
	if (ret && isAbortErr) {
		log_and_abort( "luaL_loadfile failed : %s", lua_tostring(m_L, -1) );
	}

	//assume(s_module_id == LuaModule_State_NotLoading);
	//s_module_id = module_id;

	// parse and execute!
	//   # Add '@' to front of string to inform LUA that it's a filename.
	//     (lua will even automatically use ... to abbreviate long paths!)

	if (ret == LUA_OK) {
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

		ThrowError(cvtLuaErrorToAjekError(ret));
	}

	GCUSAGE(m_L);
	// grammar checks
}

template<typename T>
void AjekScriptEnv::_check_int_trunc(T result, int stackidx, const char* funcname, const char* varname) const
{
	if (!result.isnil()) {
		s64 intval = lua_tointeger(m_L, stackidx);
		if(intval != s64(result.m_value)) {
			warn_host( "%2(\"%s\") - Integer truncation %s -> %s",
				funcname, varname, cHexStr(intval), cHexStr((s64)result.m_value));
		}
	}
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

bool AjekScriptEnv::glob_IsNil(const char* varname) const
{
	lua_getglobal(m_L, varname);
	bool result = !!lua_isnil(m_L, -1);
	lua_pop(m_L, 1);
	return result;

}

lua_s32	AjekScriptEnv::get_s32(int stackidx) const
{
	lua_s32 result;
	result.m_value		= lua_tointeger(m_L, stackidx);
	result.m_isNil		= lua_isnil(m_L, stackidx);
	return result;
}

lua_s32	AjekScriptEnv::glob_get_s32(const char* varname) const
{
	lua_getglobal(m_L, varname);
	auto result = get_s32();

	if (!result.isnil() && !lua_isinteger(m_L, -1)) {
		_throw_type_mismatch(varname, "s32");
	};

	_check_int_trunc(result, -1, "glob_get_s32", varname);
	lua_pop(m_L, 1);
	return result;
}

lua_bool AjekScriptEnv::glob_get_bool(const char* varname) const
{
	lua_bool result;
	lua_getglobal(m_L, varname);
	result.m_isNil = lua_isnil(m_L, -1);

	if (!result.isnil() && !lua_isboolean(m_L, -1)) {
		_throw_type_mismatch(varname, "bool");
	};
	result.m_value = lua_toboolean(m_L, -1);
	lua_pop(m_L, 1);
	return result;
}

lua_string AjekScriptEnv::glob_get_string(const char* varname) const
{
	lua_string	result;

	lua_getglobal(m_L, varname);
	result.m_isNil = lua_isnil(m_L, -1);
	result.m_value = lua_tostring(m_L, -1);

	if (!result.m_value) {
		bug_on(!result.m_isNil);
		result.m_value = "";
	}
	lua_pop(m_L, 1);
	return result;
}

void AjekScriptEnv::SetJmpCatch(jmp_buf& buf) {
	bug_on_qa(m_has_setjmp);
	m_jmpbuf		= &buf;
	m_has_setjmp	= 1;

	m_L->jmpbuf_default = m_jmpbuf;
}

void AjekScriptEnv::SetJmpFinalize() {
	m_L->jmpbuf_default = nullptr;
	m_has_setjmp		= 0;
}

void AjekScriptEnv::RethrowError()
{
	ThrowError(cvtLuaErrorToAjekError(m_L->errorStatus));
}

bool AjekScriptEnv::ThrowError(AjekScriptError errorcode) const
{
	// For general semantics of this class, it's important to have the ThrowError()
	// callable from otherwise const functions.  m_error has been marked volatile
	// accordingly.

	const_cast<AjekScriptEnv*>(this)->m_error = errorcode;

	if (m_has_setjmp) {
		// Do not log last error here --
		//    assume the error handler may want to do it's own error msg printout
		longjmp(*m_jmpbuf, 1);
	}
	else {
		PrintLastError();
	}

	// always return false, this is just a convenience function for friendly semantics caller-side
	return false;
}

LuaTableScope AjekScriptEnv::glob_open_table(const char* tableName, bool isRequired)
{
	lua_getglobal(m_L, tableName);

	LuaTableScope result = LuaTableScope(*this);

	if (isRequired && result.isNil()) {
		lua_pushfstring(m_L, "Required global table '%s' is missing.", tableName);
		ThrowError(AsError_Environment);
	}

	if (!result.isNil() && !lua_istable(m_L, -1)) {
		lua_pushfstring(m_L, "Required global table '%s' is missing.", tableName);
		ThrowError(AsError_Environment);
	}
	return result;
}

LuaTableScope::~LuaTableScope() throw()
{
	if (m_env) {
		// Perform a stack check against when the table was opened.
		lua_settop(m_env->m_L, m_top);
	}
	m_env = nullptr;
}

LuaTableScope::LuaTableScope(LuaTableScope&& rvalue) {
	auto&& dest = std::move(*this);
	xObjCopy(dest, rvalue);
	rvalue.m_env = nullptr;		// disabled destructor.
}

// Returns FALSE if varname is either nil or not a table.  It is considered the responsibility of the
// caller to use glob_IsNil() check ahead of this call if they want to implement special handling of
// nil separate from non-table-type behavior.
LuaTableScope::LuaTableScope(AjekScriptEnv& env)
{
	m_env	= &env;
	m_isNil	= lua_isnil(m_env->m_L, -1);
	m_top	= lua_gettop(m_env->m_L) - 1;
}

void AjekScriptEnv::_throw_type_mismatch(const char* key, const char* expected_type) const
{
	auto* L = m_L;
	lua_pushfstring(L, "Table member '%s.%s': Expected %s but got %s.",
		"{table}", key, expected_type, lua_typename(L, lua_type(L, -1))
	);

	ThrowError(AsError_Runtime);
}

void AjekScriptEnv::_throw_type_mismatch(int keyidx, const char* expected_type) const
{
	// this is needed for properly testing non-script stack value sites:
	pragma_todo("Make Ajek-version of lua_tointeger() that does assertion checking on type");

	auto* L = m_L;
	lua_pushfstring(L, "Table member '%s[%d]': Expected %s but got %s.",
		"{table}", keyidx, expected_type, lua_typename(L, lua_type(L, -1))
	);

	ThrowError(AsError_Runtime);
}

void LuaTableScope::_internal_gettable() const {
	auto* L = m_env->m_L;
	int topidx = m_top - lua_gettop(L);
    lua_gettable(L, topidx);
}

lua_s32 LuaTableScope::get_s32(int keyidx) const
{
	auto* L = m_env->m_L;
    lua_pushinteger(L, keyidx);
	_internal_gettable();

	lua_s32 result = m_env->get_s32();
	m_env->_check_int_trunc(result, -1, "get_s32");

	if (!result.isnil() && !lua_isinteger(L, -1)) {
		m_env->_throw_type_mismatch(keyidx, "s32");
	}

	lua_pop(L, 1);

	return result;
}

lua_s32 LuaTableScope::get_s32(const char* key) const
{
	auto* L = m_env->m_L;
    lua_pushstring(L, key);
    lua_gettable(L, -2);

	lua_s32 result = m_env->get_s32();
	m_env->_check_int_trunc(result, -1, "get_s32", key);

	if (!result.isnil() && !lua_isinteger(L, -1)) {
		m_env->_throw_type_mismatch(key, "s32");
	}

	lua_pop(L, 1);

	return result;
}


lua_bool LuaTableScope::get_bool(const char* key) const
{
	auto* L = m_env->m_L;
    lua_pushstring(L, key);
    _internal_gettable();

	lua_bool result;
	result.m_isNil = lua_isnil(L, -1);
	result.m_value = lua_toboolean(L, -1);

	// strings, ints, numbers are OK.  But make an API for it because it should be a standard rule...
	pragma_todo("Relax bool type checking here to include non-table values.");
	if (!result.isnil() && !lua_isboolean(L, -1)) {
		m_env->_throw_type_mismatch(key, "bool");
	}

	lua_pop(L, 1);
	return result;
}

lua_string LuaTableScope::_impl_get_string() const
{
	auto* L = m_env->m_L;
    _internal_gettable();

	lua_string	result;
	result.m_isNil = lua_isnil(L, -1);
	result.m_value = lua_tolstring(L, -1, &result.m_length);

	if (!result.m_value) {
		bug_on(!result.m_isNil);
		result.m_value = "";
	}

	lua_pop(L, 1);
	return result;
}

lua_string LuaTableScope::_impl_conv_string()
{
	auto* L = m_env->m_L;
    _internal_gettable();

	// lua_tostring()   modifies the table by converting integers to strings (generally not good for us)
	// lua_tolstring()  pushes a new string onto heap/stack, and also resolves __string and __name meta-table refs.  Good!

	//   Since lua_tolstring() pushes a new value onto the stack, it becomes necessary to unwind
	//   the part of the stack that has our table info in it, and re-push it after the tolstring reference.
	//   (note: simply re-pushing table info is OK, unless we expected to be running hundreds of these
	//    operations during a single C call invocation.  Excess stack contents are discarded on return
	//    from pcall and after APIs finish walking tables, etc).

	lua_string	result;
	result.m_isNil	= lua_isnil(L, -1);
	result.m_value	= luaL_tolstring(L, -1, &result.m_length);

	if (!result.m_value) {
		bug_on(!result.m_isNil);
		result.m_value = "";
	}

	//lua_pop(L, 1);		// do not call?

	return result;
}

lua_string LuaTableScope::get_string(int keyidx) const
{
	auto* L = m_env->m_L;
    lua_pushinteger(L, keyidx);
	return _impl_get_string();
}

lua_string LuaTableScope::get_string(const char* key) const
{
	auto* L = m_env->m_L;
    lua_pushstring(L, key);
	return _impl_get_string();
}

LuaTableScope LuaTableScope::get_table(const char* key)
{
	auto* L = m_env->m_L;
    lua_pushstring(L, key);
    _internal_gettable();

	LuaTableScope result (*m_env);

	if (!result.isNil() && !lua_istable(L, -1)) {
		m_env->_throw_type_mismatch(key, "table");
	};
	return result;

}

LuaFuncScope LuaTableScope::push_func(const char* key)
{
	auto* L = m_env->m_L;
    lua_pushstring(L, key);
    _internal_gettable();

	bug_on (!lua_isfunction(L, -1));

	return LuaFuncScope(m_env);
}

void LuaTableScope::PrintTable()
{
	auto* L = m_env->m_L;
	lua_pushnil(L);
	while(lua_next(L, -2)) {
		const char* val = lua_tostring(L, -1);
		const char* key = lua_tostring(L, -2);

		log_host( "%s = %s", key, val );
		lua_pop(L, 1);
	}
}


void AjekScriptEnv::pushvalue(const xString& string)
{
	lua_pushstring(m_L, string);
}

void AjekScriptEnv::pushvalue(const float& number)
{
	lua_pushnumber(m_L, number);
}

void AjekScriptEnv::pushvalue(const lua_func& function)
{
	lua_pushcfunction(m_L, function.m_value);
}

void AjekScriptEnv::pushvalue(s64 integer)
{
}


template<> u32	AjekScriptEnv::to(int idx) const
{
	return lua_tointeger(m_L, idx);
}

template<> s32	AjekScriptEnv::to(int idx) const
{
	return lua_tointeger(m_L, idx);
}

template<> s64	AjekScriptEnv::to(int idx) const
{
	return lua_tointeger(m_L, idx);
}

void AjekScriptEnv::pop(int num)
{
	lua_pop(m_L, num);
}

bool AjekScriptEnv::call(int nArgs, int nRet)
{
	int result = lua_pcall(m_L, nArgs, nRet, 0);
	if (result) {
		ThrowError(cvtLuaErrorToAjekError(result));
	}
	return true;
}

LuaFuncScope::LuaFuncScope(LuaFuncScope&& rvalue) {
	auto&& dest = std::move(*this);
	xObjCopy(dest, rvalue);
	rvalue.m_env = nullptr;		// disabled destructor.
}

bool LuaFuncScope::execcall(int nRet) {
	bug_on(!m_env);
	m_env->call(m_numargs, nRet);
	m_cur_retval = 0;
	return true;
}

LuaFuncScope::LuaFuncScope(AjekScriptEnv& env) {
	m_env = &env;

}

LuaFuncScope::LuaFuncScope(AjekScriptEnv* env) {
	m_env   = env;
	m_isNil = lua_isnil(m_env->m_L, -1);
}

LuaFuncScope::~LuaFuncScope() throw() {
	if (m_env) {
		m_env->pop(m_cur_retval);
		m_env = nullptr;
	}
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