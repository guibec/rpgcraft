
#include "x-types.h"
#include "x-stl.h"
#include "x-stdlib.h"
#include "x-string.h"

#include "x-assertion.h"
#include "x-thread.h"

#include "ajek-script.h"
#include "ajek-script-internal.h"

extern "C" {
#   include "linit.c"
}

DECLARE_MODULE_NAME("lua-main");
DECLARE_MODULE_THROW(xThrowModule_Script);

AjekScriptSettings      g_ScriptConfig;
AjekScriptTraceSettings g_ScriptTrace;
AjekScriptEnv           g_scriptEnv;

static xString  s_script_dbg_path_prefix;

static AjekScriptError cvtLuaErrorToAjekError(int lua_error)
{
    switch (lua_error) {
        case LUA_YIELD      :   bug("LUA_YIELD.  HOW??");                       break;
        case LUA_ERRRUN     :   return AsError_Runtime;                         break;
        case LUA_ERRSYNTAX  :   return AsError_Syntax;                          break;
        case LUA_ERRMEM     :   bug_qa("Lua Out Of Memory Error!");             break;
        case LUA_ERRGCMM    :   bug_qa("Lua Garbage Collector Failure!");       break;
        case LUA_ERRERR     :   return AsError_Assertion;                       break;
        case LUA_ERRFILE    :   return AsError_Syntax;                          break;
        default             :   bug_qa("Lua unknown error code = %d", lua_error);
    }

    return AsError_Environment;
}

bool AjekScript_LoadConfiguration(AjekScriptEnv& env)
{
    if (auto& scriptConfig = env.glob_open_table("ScriptConfig")) {
        auto old_script_config = g_ScriptConfig;
        g_ScriptConfig.path_to_modules   = scriptConfig.get_string  ("ModulePath");
        g_ScriptConfig.lua_print_enabled = scriptConfig.get_bool    ("LuaPrintEnable");

        if (g_ScriptConfig.path_to_modules != old_script_config.path_to_modules) {
            log_host_loud("   > ModulePath = %s", g_ScriptConfig.path_to_modules.c_str());
        }
        if (!g_ScriptConfig.lua_print_enabled || (g_ScriptConfig.lua_print_enabled != old_script_config.lua_print_enabled)) {
            log_host_loud("   > Lua Print has been turned %s!", g_ScriptConfig.lua_print_enabled ? "ON" : "OFF");
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
        lua_getinfo (m_L, "lS",     &info1);

        lua_Debug info0;
        lua_getstack(m_L, level-1,  &info0);
        lua_getinfo (m_L, "n",      &info0);

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
    lua_getstack(L, 0,      &info0);
    lua_getinfo (L, "nlS",  &info0);

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

    warn_host("%s(%d): WARN: new global variable created", info0.short_src, info0.currentline);

    // log function name in future?
    //info0.name ? info0.name : "[module]", info0.name ? "()" : "" );
}

/* number of chars of a literal string without the ending \0 */
#define LL(x)           (sizeof(x)/sizeof(char) - 1)
#define addstr(a,b,l)   ( memcpy(a,b,(l) * sizeof(char)), a += (l) )
#define RETS            "..."

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
    _host_log_v(xLogFlag_Important, "Lua", fmt, list);
    flush_log();
    va_end(list);
}

extern "C" void ajek_lua_abort()
{
    x_abort( "lua aborted!" );

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

void AjekScript_InitAlloc()
{
    // generate mspaces here, in the future.
}

void AjekScript_InitGlobalEnviron()
{
    g_ScriptConfig.SetDefaultState  ();
    g_ScriptTrace.SetDefaultState   ();
    g_scriptEnv.Alloc();
}

void AjekScriptEnv::Alloc()
{
    // Future MSPACE registration goes here.
}

static int lua_panic(lua_State* L)
{

    lua_getglobal(L, "AjekScriptThisPtr");
    AjekScriptEnv* env = (AjekScriptEnv*)lua_tointeger(L, -1);
    lua_pop(L, 1);
    x_abort_on(!env);

    env->m_lua_error = cvtLuaErrorToAjekError(L->status);
    const char* err_msg = lua_tostring(L, -1);
    throw_abort_ex(env->m_lua_error, "%s", err_msg);
    return 0;
}

// The default lua library version of `print` is problematic for fowllowing reasons:
//  * it uses the same lua_writestring() as the rest of the lua engine core, even though it
//    always writes text with newlines, and batch-writes a bunch of lines at a time (one per
//    parameter).
//  * it delimits using tab character and there's barely any useful reason to ever do that.
//
// Replaced it with a `print` that:
//  * delimits by newline and adds grep-able module-name prefix
//  * provides mutex locking around entire set of lines being printed

static int ajek_luaB_print (lua_State *L) {

    // hmm... this also bypasses tostring checks on parameters... but do we care if we're not printing?
    if (!g_ScriptConfig.lua_print_enabled) return 0;

    int n = lua_gettop(L);  /* number of arguments */
    int i;

    lua_getglobal(L, "AjekScriptThisPtr");
    auto* ajek = (AjekScriptEnv*)lua_topointer(L, -1);
    bug_on(!ajek);

    ajek->m_log_buffer.Clear();

    // obtaining module name is tricky and slow using vanilla lua:  one must walk up the stack until
    // a valid module name is spotted, using lua_getstack() and lua_getinfo().  Can't make any assumptions
    // that the stack entry directly above us is a valid lua module-scope stack.
    //
    // A more efficient method would be to use C closure upvalues and bind a custom `print` closure for
    // every module that's loaded by the engine.  Caveat: modules loaded by `require` would default to
    // using the closure bound whenever the first `require` instance is encountered.
    //
    // Alternative: use a global variable _G.Ajek.PrintPrefix, which is set up when modules are loaded,
    // and can be overridden from inside the module too, if desired.  Ok yea, I like that one!  --jstine

    pragma_todo("Use current lua module name as `print` prefix, via global (see comment)");

    lua_getglobal(L, "tostring");
    for (i=1; i<=n; i++) {
        const char *s;
        size_t l;
        lua_pushvalue(L, -1);  /* function to be called */
        lua_pushvalue(L, i);   /* value to print */
        lua_call(L, 1, 1);
        if (s = lua_tolstring(L, -1, &l)) {
            if (i>1) ajek->m_log_buffer += "\n";
            ajek->m_log_buffer.AppendFmt("%-20s: ", "Lua");
            ajek->m_log_buffer.Append(s, l);
        }
        else {
            return luaL_error(L, "'tostring' must return a string to 'print'");
        }
        lua_pop(L, 1);  /* pop result */
    }
    xPrintLn(ajek->m_log_buffer);
    return 0;
}

void AjekScriptEnv::NewState()
{
    DisposeState();

    m_L         = luaL_newstate();
    m_lua_error = AsError_None;

    x_abort_on( !m_L, "Create new Lua state failed." );
    log_host( "luaState = %s", cPtrStr(m_L) );

    // store this ajekscript handle in the lua heap for error handling (not really meant to be
    // referenced from Lua) -- might want make a custom registry in the future if more vars are
    // added.  See LUA_RIDX_LAST.

    lua_pushlightuserdata   (m_L, this);
    lua_setglobal           (m_L, "AjekScriptThisPtr");
    lua_atpanic             (m_L, &lua_panic);

    luaL_openlibs           (m_L);
    lua_pushcfunction       (m_L, ajek_luaB_print);
    lua_setglobal           (m_L, "print");

}

void AjekScriptEnv::DisposeState()
{
    if (!m_L)       return;

    // Wipes entire Lua state --- should delete and re-create MSPACE as well.
    log_host( "Disposing AjekScript Environment...");

    lua_close(m_L);
    m_L         = nullptr;
}


void AjekScriptEnv::LoadModule(const xString& path)
{
    bug_on (!m_L,                       "Invalid object state: uninitialized script environment.");
    bug_on (path.IsEmpty());

    if (m_lua_error != AsError_None)    return;
    if (path.IsEmpty())                 return;

    //ScopedFpRoundMode nearest;

    log_host( "LoadModule: %s", path.c_str() );

    // TODO: register init-only global C functions here ...
    // [...]
    // END

    int ret;
    ret     = luaL_loadfile(m_L, path);

    if (ret) {
        bool isAbortErr = ret != LUA_ERRSYNTAX && ret != LUA_ERRFILE;
        if (isAbortErr) {
            x_abort( "luaL_loadfile failed : %s", lua_tostring(m_L, -1) );
        }
        m_lua_error = cvtLuaErrorToAjekError(ret);
        throw_abort_ex(AsError_Syntax, lua_tostring(m_L, -1));
    }

    //assume(s_module_id == LuaModule_State_NotLoading);
    //s_module_id = module_id;

    // parse and execute!
    //   # Add '@' to front of string to inform LUA that it's a filename.
    //     (lua will even automatically use ... to abbreviate long paths!)

    if (ret == LUA_OK) {
        //if (trace_libajek) {
        //  log_tooling("[FnTrace] ExecModule(\"%s\")", filename);
        //  s_fntrace_nesting += '>';
        //}

        if (m_ThrowCtx && m_ThrowCtx->CanThrow()) {
            // errors arrive in owner's exception handler
            lua_call(m_L, 0, 0);
        }
        else {
            // errors recorded into
            ret = lua_pcall(m_L, 0, 0, 0);
            if (ret) {
                m_lua_error = cvtLuaErrorToAjekError(ret);
            }
        }

        //if (trace_libajek) {
        //  s_fntrace_nesting.PopBack();
        //}
    }

    // Cleanup!
    //  # Free memory
    //  # remove init-only APIs.

    // just for low-level debugging...
    //x_abort_on(ret, "script load error.\n%s", lua_tostring(m_L, -1) );
    GCUSAGE(m_L);
}

// Use macros to throw errors so that file/funcname information is more useful and accurate --jstine
#define _throw_type_mismatch(env, key, expected_type)                                           \
    throw_abort_ex(AsError_Runtime, "Table member '%s.%s': Expected %s but got %s.",            \
        "{table}", key, expected_type, lua_typename((env)->m_L, lua_type((env)->m_L, -1))       \
    );

#define _throw_type_mismatch_idx(env, keyidx, expected_type)                                    \
    throw_abort_ex(AsError_Runtime, "Table member '%s[%d]': Expected %s but got %s.",           \
        "{table}", keyidx, expected_type, lua_typename((env)->m_L, lua_type((env)->m_L, -1))    \
    );

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

bool AjekScriptEnv::glob_IsNil(const char* varname) const
{
    lua_getglobal(m_L, varname);
    bool result = !!lua_isnil(m_L, -1);
    lua_pop(m_L, 1);
    return result;

}

lua_s32 AjekScriptEnv::get_s32(int stackidx) const
{
    lua_s32 result;
    result.m_value      = lua_tointeger(m_L, stackidx);
    result.m_isNil      = lua_isnil    (m_L, stackidx);
    return result;
}

lua_s32 AjekScriptEnv::glob_get_s32(const char* varname) const
{
    lua_getglobal(m_L, varname);
    auto result = get_s32();

    if (!result.isnil() && !lua_isinteger(m_L, -1)) {
        _throw_type_mismatch(this, varname, "s32");
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
        _throw_type_mismatch(this, varname, "bool");
    };
    result.m_value = lua_toboolean(m_L, -1);
    lua_pop(m_L, 1);
    return result;
}

lua_string AjekScriptEnv::glob_get_string(const char* varname) const
{
    lua_string  result;

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


static void lua_custom_throw(int lua_error)
{

    cvtLuaErrorToAjekError(lua_error);
}

void AjekScriptEnv::BindThrowContext(xThrowContext& ctx)
{
    // don't think it makes sense to bug here .. it's not really a vector for hard-to-track
    // buggy behavior.  --jstine

    //bug_on_qa(m_ThrowCtx);
    m_ThrowCtx = &ctx;
}

LuaTableScope AjekScriptEnv::glob_open_table(const char* tableName, bool isRequired)
{
    lua_getglobal(m_L, tableName);

    LuaTableScope result = LuaTableScope(*this);

    if (isRequired && result.isNil()) {
        throw_abort_ex(AsError_Environment, "Required global table '%s' is missing.", tableName);
    }

    if (!result.isNil() && !lua_istable(m_L, -1)) {
        throw_abort_ex(AsError_Environment, "Required global table '%s' is missing.", tableName);
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
    rvalue.m_env = nullptr;     // disabled destructor.
}

// Returns FALSE if varname is either nil or not a table.  It is considered the responsibility of the
// caller to use glob_IsNil() check ahead of this call if they want to implement special handling of
// nil separate from non-table-type behavior.
LuaTableScope::LuaTableScope(AjekScriptEnv& env)
{
    m_env   = &env;
    m_isNil = lua_isnil(m_env->m_L, -1);
    m_top   = lua_gettop(m_env->m_L) - 1;
}

// this is needed for properly testing non-script stack value sites:
pragma_todo("Make Ajek-version of lua_tointeger() that does assertion checking on type");

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
        _throw_type_mismatch_idx(m_env, keyidx, "s32");
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
        _throw_type_mismatch(m_env, key, "s32");
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
        _throw_type_mismatch(m_env, key, "bool");
    }

    lua_pop(L, 1);
    return result;
}

lua_string LuaTableScope::_impl_get_string() const
{
    auto* L = m_env->m_L;
    _internal_gettable();

    lua_string  result;
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

    lua_string  result;
    result.m_isNil  = lua_isnil(L, -1);
    result.m_value  = luaL_tolstring(L, -1, &result.m_length);

    if (!result.m_value) {
        bug_on(!result.m_isNil);
        result.m_value = "";
    }

    //lua_pop(L, 1);        // do not call?

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
        _throw_type_mismatch(m_env, key, "table");
    };
    return result;

}

LuaTableScope LuaTableScope::get_table(int keyidx)
{
    auto* L = m_env->m_L;
    lua_pushinteger(L, keyidx);
    _internal_gettable();

    LuaTableScope result (*m_env);

    if (!result.isNil() && !lua_istable(L, -1)) {
        _throw_type_mismatch_idx(m_env, keyidx, "table");
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


void AjekScriptEnv::pushreg(const AjekReg_Closure& closure)
{
    lua_rawgeti(m_L, LUA_REGISTRYINDEX, closure.m_regkey);
    bug_on(!lua_isfunction(m_L, -1), "Invalid closure key.");
}

void AjekScriptEnv::pushreg(const AjekReg_Table& table)
{
    lua_rawgeti(m_L, LUA_REGISTRYINDEX, table.m_regkey);
    bug_on(!lua_istable(m_L, -1), "Invalid table key.");
}

void AjekScriptEnv::pushvalue(const xString& string)
{
    lua_pushstring(m_L, string);
}

void AjekScriptEnv::pushvalue(const float& number)
{
    lua_pushnumber(m_L, number);
}

void AjekScriptEnv::pushvalue(const lua_cfunc& function)
{
    lua_pushcfunction(m_L, function.m_value);
}

void AjekScriptEnv::pushvalue(s64 integer)
{
}


template<> u32  AjekScriptEnv::to(int idx) const
{
    return lua_tointeger(m_L, idx);
}

template<> s32  AjekScriptEnv::to(int idx) const
{
    return lua_tointeger(m_L, idx);
}

template<> s64  AjekScriptEnv::to(int idx) const
{
    return lua_tointeger(m_L, idx);
}

void AjekScriptEnv::pop(int num)
{
    lua_pop(m_L, num);
}

bool AjekScriptEnv::call(int nArgs, int nRet)
{
    //int result = lua_pcall(m_L, nArgs, nRet, 0);
    //if (result) {
    //  ThrowError(cvtLuaErrorToAjekError(result));
    //}
    lua_call(m_L, nArgs, nRet);
    return true;
}

LuaFuncScope::LuaFuncScope(LuaFuncScope&& rvalue) {
    auto&& dest = std::move(*this);
    xObjCopy(dest, rvalue);
    rvalue.m_env = nullptr;     // disabled destructor.
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

//  env.table_get_string("ModulePath");

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