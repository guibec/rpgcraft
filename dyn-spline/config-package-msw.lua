
-- DEVELOPERS: DO NOT MODIFY.
-- Create and modify config-local.lua (not checked into git), and override settings from there.

-- This configuration is provided by git repo, and applies default settings according
-- to the target platform.  

ScriptConfig = {}

ScriptConfig.IncludePaths 		= { "./", 	"./lua_req"		} 
ScriptConfig.DebugIncludePaths	= { "./", 	"./lua_dbg_req"	}
ScriptConfig.ModulePath			= "./lua_modules"

ScriptConfig.UseProtectedCall	= true
ScriptConfig.ScriptDebugging    = true
ScriptConfig.LuaPrintEnable		= true

-- Read in local developer configuration settings.
load("config-local.lua")
