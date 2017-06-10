
-- Apply platform/package default settings

ScriptConfig = {}

ScriptConfig.IncludePaths 		= { "./", 	"./lua_req"		} 
ScriptConfig.DebugIncludePaths	= { "./", 	"./lua_dbg_req"	}
ScriptConfig.ModulePath			= "./lua_modules"

ScriptConfig.UseProtectedCall	= true
ScriptConfig.EnableScriptDebug  = true


-- Read in local developer configuration settings.
load("config-local.lua")
