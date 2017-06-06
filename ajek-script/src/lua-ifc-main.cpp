
#include "x-types.h"
#include "x-stl.h"
#include "x-stdlib.h"
#include "x-string.h"

#include "x-assertion.h"

#include "ajek-script.h"
#include "lua-ifc-internal.h"

extern "C" {
#	include "linit.c"
}

DECLARE_MODULE_NAME("lua-main");

bool g_script_log_verbose	= 1;

extern "C" void ajek_lua_printf(const char *fmt, ...)
{
	va_list list;
	va_start(list, fmt);
	if (g_script_log_verbose) {
		_host_log_v(xLogFlag_Default, "Lua", fmt, list);
		if (!TARGET_CONSOLE) {
			// to support 'tail' utils via CLI
			// limited to non-console builds to avoid potential perf impacts, and on orbis/xbox
			// use of the orbis console stream should be preferred.
			flush_log();
		}
	}
	va_end(list);
}

extern "C" void ajek_lua_abort()
{
	log_and_abort( "lua aborted!" );

	// abort is technically unreachable -- just here for __noreturn adherence, since log_abort()
	// is not always marked as noreturn depending on the build target configuration.
	if (!xIsDebuggerAttached()) {
		abort();
	}
}

// Goals:
//  1. create module for engine parameter setup.   Debug settings, running environment, paths, so on.
//  2. create module for specifying behavior of tile engine things.
//       tilesets to load.
//       starting position.
//       response to user input.