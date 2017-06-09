
#pragma once

#include "x-types.h"

extern "C" {
#include "lua.h"
#include "lauxlib.h"
#include "lstate.h"
}


#if 0
#	define GCUSAGE(LuaStack)	\
	do {						\
		if (LuaStack) { \
			auto gc_mem_usage = ((uint64_t)lua_gc(LuaStack, LUA_GCCOUNT, 0) << 10) + lua_gc(LuaStack, LUA_GCCOUNTB, 0); \
			log_host("mem usage: %d Bytes @ %s %s:%d", gc_mem_usage, __FUNCTION_NAME__, __FILE__, __LINE__); \
		} \
	} while (0)
#else
#	define GCUSAGE(LuaStack)
#endif
