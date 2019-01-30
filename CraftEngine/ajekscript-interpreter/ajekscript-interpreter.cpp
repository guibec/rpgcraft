
#include "x-types.h"
#include "x-string.h"
#include "x-assertion.h"
#include "x-thread.h"

#include <cstdio>
#include <cstdlib>
#include <cstdarg>
#include <intrin.h>

extern "C" {
#   include "lua.h"
}


/* number of chars of a literal string without the ending \0 */
#define LL(x)           (sizeof(x)/sizeof(char) - 1)
#define addstr(a,b,l)   ( memcpy(a,b,(l) * sizeof(char)), a += (l) )
#define RETS            "..."

static void _debugBreakContext( xString& context, const AssertContextInfoTriad& triad )
{

    context.Format(
        "Condition  :  %s\n"
        "Function   :  %s\n",

        triad.cond ? triad.cond : "(none)",
        triad.funcname
    );
}

static void _flush_all_that_filesystem_jazz()
{
    fflush(nullptr);
}

assert_t xDebugBreak_v( DbgBreakType breakType, const AssertContextInfoTriad& triad, const char* fmt, va_list list )
{
    static __threadlocal int s_recursion_guard = 0;
    xScopedIncremental  guard( s_recursion_guard );

    if (!guard.TryEnter())
    {
        __debugbreak();
        exit(66);
    }

    xString context;
    _debugBreakContext( context, triad );

    xString message;
    if (fmt) {
        message.FormatV(fmt, list);
    }

    xPrintLn( xFmtStr("%s%s\n\nContext:\n%s", triad.filepos, message.c_str(), context.c_str()) );
    _flush_all_that_filesystem_jazz();
    return assert_break;
}

void _host_log(uint flags, const char* moduleName, const char* fmt, ...)
{
    if (fmt && fmt[0])
    {
        va_list list;
        va_start(list, fmt);
        vprintf(fmt, list);
        va_end(list);
        printf("\n");
    }
}

void xPrintLn(const xString& msg)
{
    printf(msg + "\n");
}


extern "C" void ajek_lua_printf(const char *fmt, ...)
{
    va_list list;
    va_start(list, fmt);
    vprintf(fmt, list);
    fflush(nullptr);
    va_end(list);
}

extern "C" void ajek_lua_abort()
{
    __debugbreak();
    abort();
}

const char* s_script_dbg_path_prefix = "./";

extern "C" void ajek_lua_ChunkId_Filename(char* out, const char* source, size_t bufflen)
{
    xString result = xFmtStr("%s/%s", s_script_dbg_path_prefix, source);

    size_t l = result.GetLength();
    if (l <= bufflen)  /* small enough? */
        memcpy(out, result.c_str(), l * sizeof(char) + 1);
    else {  /* add '...' before rest of name */
        addstr(out, RETS, LL(RETS));
        bufflen -= LL(RETS);
        memcpy(out, result.c_str() + l - bufflen, bufflen * sizeof(char) + 1);
    }
}

DECLARE_MODULE_NAME("as-int");

extern "C" void ajek_warn_new_global(lua_State* L)
{
    lua_Debug info0;
    lua_getstack(L, 0,      &info0);
    lua_getinfo (L, "nlS",  &info0);

    printf("%s(%d): WARN: new global variable created\n", info0.short_src, info0.currentline);

    // log function name in future?
    //info0.name ? info0.name : "[module]", info0.name ? "()" : "" );
}
