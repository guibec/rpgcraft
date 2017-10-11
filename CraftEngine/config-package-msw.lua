
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

Engine = {}
Engine.TileSizeX				= 8
Engine.TileSizeY				= 8

-- ------------------------------------------------------------------------------------------------
-- DevForce
--
-- Parameters provided to allow devs to hack up certain behaviors in their local running environment.
-- Values can be specified in config-local, allowing them to avoid accidentally checking these hacks
-- into git.  May also be used to define package-specific behaviors (such as iPhone deployment).

DevForce = {}

DevForce.FormFactor 		= nil			-- 'desktop','phone','tablet'

DevForce.WorldView = {}
DevForce.WorldView.MeshSize	= nil 			-- { x=nil, y=nil }
-- ------------------------------------------------------------------------------------------------

-- Read in local developer configuration settings.
load("config-local.lua")
