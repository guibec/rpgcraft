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

struct AjekScriptEnv
{
	lua_State*		m_L;
	//AjekMspace*	m_mspace;		// Future custom mspace provision

	AjekScriptEnv() {
		m_L = nullptr;
	}

	void		Alloc					();
	void		RegisterFrameworkLibs	();
	void		LoadModule				(const xString& path);

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

extern void				AjekScript_InitModuleList			();
extern lua_State*		AjekScript_GetLuaState				(ScriptEnvironId moduleId);
extern void				AjekScript_SetDebugAbsolutePaths	(const xString& cwd, const xString& target);
extern void				AjekScript_SetDebugRelativePath		(const xString& relpath);

extern AjekScriptEnv&	AjekScriptEnv_Get					(ScriptEnvironId moduleId);

