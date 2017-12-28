
#include "PCH-rpgcraft.h"

#include "x-types.h"
#include "x-stl.h"
#include "x-assertion.h"

#include "appConfig.h"
#include "ajek-script.h"


DECLARE_MODULE_NAME("config");

xString g_pkg_config_filename;

void LoadPkgConfigFromMain(AjekScriptEnv& env)
{
resumeLoop:
	g_scriptEnv.NewState();
	g_scriptEnv.BindThrowContext(g_ThrowCtx);

	x_try() {
		AjekScript_InitGlobalEnviron();
		g_scriptEnv.LoadModule(g_pkg_config_filename);
		AjekScript_LoadConfiguration(g_scriptEnv);
	}
	x_catch() {
		case xThrowModule_Script:
		case xThrowModule_Bridge:
			g_ThrowCtx.PrintLastError();
			if (!xIsDebuggerAttached()) {
				x_abort("Application aborted due to scriptConfig error.");
			}
			AjekScript_PrintBreakReloadMsg();
			__debugbreak();		// allows developer to resume after correcting errors.
		goto resumeLoop;

		default:
			x_abort("Unhandled exception type!");
		break;
	}
	x_finalize() {
	}
}
