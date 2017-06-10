
#include "x-types.h"
#include "x-stl.h"
#include "x-assertion.h"

#include "ajek-script.h"

DECLARE_MODULE_NAME("config");


void AjekScript_PrintDebugReloadMsg() {
	xPrintLn("");
	xPrintLn("Debugger detected - Correct the problem in Lua and hit 'Continue' in debugger to reload and re-execute.");
	xPrintLn("");
}

#define tryLoadLuaAgain(...) {																		\
	if (xIsDebuggerAttached())  { AjekScript_PrintDebugReloadMsg(); __debugbreak(); return false; }	\
	else						{ log_and_abort( __VA_ARGS__ ); }									\
}

static bool TryLoadPkgConfig()
{
	auto& env = AjekScriptEnv_Get(ScriptEnv_AppConfig);

	env.NewState();
	auto* L	= env.getLuaState();

	env.LoadModule("config-package.lua");

	if (env.HasError()) {
		return false;
	}

	if (1) {
		LuaTableScope scriptConfig(env, "ScriptConfig");
		if (scriptConfig.isNil()) {
			warn_host("Missing configuration table 'ScriptConfig'.");
			return false;
		}
		if (!scriptConfig.isTable()) {
			warn_host("Invalid definition of 'ScriptConfig' - it must be a table");
			return false;
		}

		auto path   = scriptConfig.get_string("ModulePath");
		log_host( "Path Test = %s", path.c_str() );
	}
	return true;
}

void LoadPkgConfig()
{
	AjekScript_InitSettings();
	AjekScript_InitModuleList();

	while (!TryLoadPkgConfig()) {
		if (!xIsDebuggerAttached()) {
			log_and_abort("Application aborted due to scriptConfig error."); 
		}
		AjekScript_PrintDebugReloadMsg();
		__debugbreak();		// allows developer to resume after correcting errors.
	}
}
