
-- DEVELOPERS: DO NOT MODIFY.
-- Create and modify config-local.lua (not checked into git), and override settings from there.

-- This configuration is provided by git repo, and applies default settings according
-- to the target platform.  

ScriptConfig = {}
ScriptConfig.ModulePath			= "./lua_modules"
ScriptConfig.UseProtectedCall	= true
ScriptConfig.ScriptDebugging    = true
ScriptConfig.LuaPrintEnable		= true

--[[
-- TODO : switch to a functional method of applying settings, rather than data-driven?
SetScriptParams({
	-- IncludePaths		= { "./", 	"./lua_req"		},
	-- DebugIncludePaths   = { "./", 	"./lua_dbg_req"	},
	ModulePath			= "./lua_modules",
	UseProtectedCall	= true,
	ScriptDebugging     = true,
	LuaPrintEnable		= true
})
-- ]]

-- Read in settings saved during previous run
-- (this step would likely not be included on console editions)
pcall(function () require("saved-settings") end)

-- Read in local developer configuration settings.
pcall(function () require("config-local") end)

