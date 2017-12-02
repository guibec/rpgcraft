#pragma once

#include "x-types.h"
#include "x-string.h"
#include "x-ThrowContext.h"

#include "x-MemCopy.inl"

extern "C" {
#	include "lua.h"
}

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

struct lua_s64 {
	s64		m_value;
	bool	m_isNil;

	bool isnil() const {
		return m_isNil;
	}

	operator s64() const {
		return m_value;
	}
};

struct lua_s32 {
	s32		m_value;
	bool	m_isNil;

	bool isnil() const {
		return m_isNil;
	}

	operator s32() const {
		return m_value;
	}
};

struct lua_u32 {
	u32		m_value;
	bool	m_isNil;

	bool isnil() const {
		return m_isNil;
	}

	operator u32() const {
		return m_value;
	}
};

struct lua_float {
	float	m_value;
	bool	m_isNil;

	bool isnil() const {
		return m_isNil;
	}

	operator float() const {
		return m_value;
	}
};

struct lua_bool {
	s32		m_value;
	bool	m_isNil;

	bool isnil() const {
		return m_isNil;
	}

	operator bool() const {
		return m_value;
	}
};

struct lua_string {
	const char*	m_value;
	size_t		m_length;
	bool		m_isNil;

	bool isnil() const {
		return m_isNil;
	}

	operator const char*() const {
		return m_value;
	}

	const char* c_str() const {
		return m_value;
	}
};

inline xString&	xString::operator=( const lua_string& lua_str ) {
	m_string.assign(lua_str.c_str(), lua_str.m_length);
	return *this;
}

inline xString::xString(const lua_string& lua_str)	: m_string(lua_str.c_str(), lua_str.m_length) { }
//inline xString::xString(lua_string&& lua_str)	: m_string(lua_str.c_str(), lua_str.m_length) { }

struct lua_cfunc {
	lua_CFunction	m_value;
	bool			m_isNil;

	bool isnil() const {
		return m_isNil;
	}
};

struct AjekReg_Closure {
	int				m_regkey;				// registry key for the closure
};

struct AjekReg_Table {
	int				m_regkey;				// registry key for the table
};

struct AjekScriptEnv;

class LuaFuncScope
{
	NONCOPYABLE_OBJECT(LuaFuncScope);

public:
	AjekScriptEnv*	m_env;					// also contains lua_State
	int				m_cur_retval= 0;		// current return value being read
	bool			m_isNil		= true;
	int				m_numargs	= 0;		// number of argument pushes to pop after finished.

	explicit LuaFuncScope(AjekScriptEnv& env);
	explicit LuaFuncScope(AjekScriptEnv* env);
	LuaFuncScope(LuaFuncScope&& rvalue);
	~LuaFuncScope() throw();


	operator bool() const {
		return !m_isNil;
	}


public:
	template< typename T >
	void			pusharg			(const T& value);
	template< typename T >
	T				getresult		();

	bool			execcall		(int nRet=0);

};

#define AsApi_Required		(true)			// must not be 'nil' !
#define AsApi_Optional		(false)			// 'nil' is allowed

class LuaTableScope
{
	NONCOPYABLE_OBJECT(LuaTableScope);

public:
	// TODO: Add LuaTableScope for opening a non-global table.
	//    (function parameter or return value, etc)

	explicit LuaTableScope(AjekScriptEnv& m_env);
	LuaTableScope(LuaTableScope&& rvalue);
	~LuaTableScope() throw();


	operator bool() const {
		return !m_isNil;
	}

	AjekScriptEnv*	m_env;		// also contains lua_State
	int				m_top;
	bool			m_isNil;

protected:
	lua_string		_impl_get_string		() const;
	lua_string		_impl_conv_string		();
	void			_internal_gettable		() const;

public:
	bool			isNil		() const	{ return m_isNil;	}

	lua_s32			get_s32		(int keyidx)		const;
	lua_string		get_string	(int keyidx)		const;

	lua_u32			get_u32		(const char* key)	const;
	lua_s32			get_s32		(const char* key)	const;
	lua_s64			get_s64		(const char* key)	const;
	lua_float		get_float	(const char* key)	const;
	lua_bool		get_bool	(const char* key)	const;
	lua_string		get_string	(const char* key)	const;
	LuaTableScope	get_table	(const char* key);
	LuaTableScope	get_table	(int keyidx);

	LuaFuncScope	push_func	(const char* key);

	void			PrintTable	();
};

// AjekScriptError -
//   Thought: not sure if we need a special code for Assertions (Which are analogous to LUA_ERRERR)
enum AjekScriptError {
	AsError_None		= 0,			// no error!
	AsError_Environment = 1,			// error while reading from global environment tables
	AsError_Syntax		= 2,			// syntax error while parsing/compiling lua script
	AsError_Runtime		= 3,			// runtime error during lua script execution (eg, accessing nil value)
	AsError_Assertion	= 4,			// An error invoked by way of assert() or error()
};

struct AjekScriptEnv
{
	lua_State*			m_L;
	//AjekMspace*		m_mspace;			// Future custom mspace provision
	xThrowContext*		m_ThrowCtx;

	AjekScriptError		m_lua_error;

	xString				m_log_buffer;

volatile
	AjekScriptEnv() {
		m_L				= nullptr;
		m_ThrowCtx		= nullptr;
	}

	bool HasError() const {
		return (m_lua_error != AsError_None);
	}

	void		Alloc					();
	void		NewState				();
	void		RegisterFrameworkLibs	();
	void		LoadModule				(const xString& path);
	void		DisposeState			();
	void		PrintStackTrace			();

	lua_State*			getLuaState		();
	const lua_State*	getLuaState		() const;

// AjekScriptEnv::pushvalue() is intentionally *not* templated, as a way to provide
// clear and explicit specialization of supported incoming types.

	void			pushvalue			(const xString&		string);
	void			pushvalue			(const float&		number);
	void			pushvalue			(const lua_cfunc&	function);
	void			pushvalue			(s64				integer);

// pushreg() - pushes values onto the stack after looking them up on the registry.
// could just overload pushvalue() instead of giving these their own name, but for now
// strong differentiation seems wise.

	void			pushreg				(const AjekReg_Closure&		closure_id);
	void			pushreg				(const AjekReg_Table&		table_id);

	template<typename T>	T			to			(int stackidx)	const;

	template<>				u32			to 			(int stackidx)	const;
	template<>				s32			to			(int stackidx)	const;
	template<>				s64			to			(int stackidx)	const;
	template<>				float		to			(int stackidx)	const;
	template<>				bool		to			(int stackidx)	const;
	template<>				xString		to			(int stackidx)	const;

	bool			call			(int nArgs, int nRet);
	void			pop				(int num);

	bool			glob_IsNil		(const char* varname)	const;

	lua_s32			get_s32			(int stackidx = -1)		const;

	lua_u32			glob_get_u32	(const char* varname)	const;
	lua_s32			glob_get_s32	(const char* varname)	const;
	lua_s64			glob_get_s64	(const char* varname)	const;
	lua_float		glob_get_float	(const char* varname)	const;
	lua_bool		glob_get_bool	(const char* varname)	const;
	lua_string		glob_get_string	(const char* varname)	const;
	LuaTableScope	glob_open_table	(const char* tableName, bool isRequired = AsApi_Required);

	template<typename T>
	void			_check_int_trunc	(T result, int stackidx, const char* funcname, const char* varname = "") const;

	void			BindThrowContext(xThrowContext& ctx);
};


extern void				AjekScript_InitAlloc				();
extern void				AjekScript_InitGlobalEnviron		();
extern void				AjekScript_SetDebugAbsolutePaths	(const xString& cwd, const xString& target);
extern void				AjekScript_SetDebugRelativePath		(const xString& relpath);
extern bool				AjekScript_LoadConfiguration		(AjekScriptEnv& env);
extern void				AjekScript_PrintDebugReloadMsg		();
extern void				AjekScript_PrintBreakReloadMsg		();

extern AjekScriptSettings		g_ScriptConfig;
extern AjekScriptTraceSettings	g_ScriptTrace;
extern AjekScriptEnv			g_scriptEnv;

// Registers an error handler setjmp point.
// Returns 1 if OK to run the function in question.
// Returns 0 if the function was run and an error occurred.
#define AjekScript_SetJmpIsOK(env)	(env.m_has_setjmp = (setjmp(env.m_jmpbuf)==0))

// Supports any type also supported by underlying AjekScriptEnv::pushvalue().
// AjekScriptEnv::pushvalue() is intentionally *not* templated, as a way to provide
// clear and explicit specialization of supported incoming types.
template< typename T >
inline void LuaFuncScope::pusharg(const T& anyvalue)
{
	bug_on(!m_env);
	++m_numargs;
	return m_env->pushvalue(anyvalue);
}

template< typename T >
inline T LuaFuncScope::getresult() {
	bug_on(!m_env);
	return m_env->to<T>(-1 - m_cur_retval);
	m_cur_retval += 1;
}

template<typename T> T AjekScriptEnv::to(int stackidx) const {
	T.Unsupported_Template_Parameter();		// generates a decent compiler error! :)
}

template<> float AjekScriptEnv::to(int idx) const
{
	return lua_tonumber(m_L, idx);
}
