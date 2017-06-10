
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

static bool TryLoadPkgConfig(const xString& luaFile)
{
	auto& env = AjekScriptEnv_Get(ScriptEnv_AppConfig);

	env.NewState();
	auto* L	= env.getLuaState();

	bug_on(luaFile.IsEmpty());
	env.LoadModule(luaFile);

	if (env.HasError()) {
		return false;
	}

	if (!AjekScript_LoadConfiguration(env)) { return false; }

	return true;
}

void LoadPkgConfig(const xString& luaFile)
{
	AjekScript_InitSettings();
	AjekScript_InitModuleList();

	while (!TryLoadPkgConfig(luaFile)) {
		if (!xIsDebuggerAttached()) {
			log_and_abort("Application aborted due to scriptConfig error."); 
		}
		AjekScript_PrintDebugReloadMsg();
		__debugbreak();		// allows developer to resume after correcting errors.
	}
}
