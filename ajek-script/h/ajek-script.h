#pragma once

#include "x-types.h"

extern "C" {
#	include "lua.h"
}

#if !defined(AJEK_SCRIPT_DEBUGGER)
#	define AJEK_SCRIPT_DEBUGGER			1
#endif

enum ScriptEnvironId {
	ScriptEnv_AppConfig,
	ScriptEnv_Game,
	_ScriptEnv_Padding,
	NUM_SCRIPT_ENVIRONMENTS
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

struct AjekScriptEnv;

class LuaTableScope
{
	NONCOPYABLE_OBJECT(LuaTableScope);

public:
	// TODO: Add LuaTableScope for opening a non-global table.
	//    (function parameter or return value, etc)

	LuaTableScope(AjekScriptEnv& m_env, const char* tableName);
	~LuaTableScope() throw();

	AjekScriptEnv*	m_env;		// also contains lua_State
	bool			m_isNil;
	bool			m_isTable;

	bool			isNil		() const	{ return m_isNil;	}
	bool			isTable		() const	{ return m_isTable; }

	lua_u32			get_u32		(const xString& key)	const;
	lua_s32			get_s32		(const xString& key)	const;
	lua_s64			get_s64		(const xString& key)	const;
	lua_float		get_float	(const xString& key)	const;
	lua_bool		get_bool	(const xString& key)	const;
	lua_string		get_string	(const xString& key);
};

struct AjekScriptEnv
{
	lua_State*		m_L;
	//AjekMspace*	m_mspace;		// Future custom mspace provision

	bool			m_has_error;
	xString			m_open_global_table;

	AjekScriptEnv() {
		m_L			= nullptr;
		m_has_error	= false;
	}

	bool HasError() const {
		return m_has_error;
	}

	void		Alloc					();
	void		NewState				();
	void		RegisterFrameworkLibs	();
	void		LoadModule				(const xString& path);
	void		DisposeState			();
	void		PrintStackTrace			();
	void		PrintLastError			() const;

	lua_State*			getLuaState		();
	const lua_State*	getLuaState		() const;

	bool			glob_IsNil		(const xString& varname)	const;
	lua_u32			glob_get_u32	(const xString& varname)	const;
	lua_s32			glob_get_s32	(const xString& varname)	const;
	lua_s64			glob_get_s64	(const xString& varname)	const;
	lua_float		glob_get_float	(const xString& varname)	const;
	lua_bool		glob_get_bool	(const xString& varname)	const;
	lua_string		glob_get_string	(const xString& varname)	const;
};

extern void				AjekScript_InitSettings				();
extern void				AjekScript_InitModuleList			();
extern lua_State*		AjekScript_GetLuaState				(ScriptEnvironId moduleId);
extern void				AjekScript_SetDebugAbsolutePaths	(const xString& cwd, const xString& target);
extern void				AjekScript_SetDebugRelativePath		(const xString& relpath);

extern AjekScriptEnv&	AjekScriptEnv_Get					(ScriptEnvironId moduleId);

