
#include "x-types.h"
#include "x-stl.h"
#include "x-assertion.h"

#include "ajek-script.h"

DECLARE_MODULE_NAME("config");

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
		AjekScript_PrintBreakReloadMsg();
		__debugbreak();		// allows developer to resume after correcting errors.
	}
}
