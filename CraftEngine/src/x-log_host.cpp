#include "PCH-rpgcraft.h"

#include "x-types.h"
#include "x-string.h"
#include "x-thread.h"
#include "x-stdfile.h"
#include "x-assertion.h"

#include "x-chrono.h"


// s_mylog_MaxFileSize
//  Above this length, the current logfile is closed and a new one is opened for continue log
//  generation.  Some editors can handle larger files (1-2gb is commonl allowable) but they
//  tend to handle those sizes poorly -- especially when doing diffs.  So splitting them up
//  usually is a low-hassle way to speed up viewing/diffing. --jstine
//
const           s64             s_mylog_MaxFileSize = _256mb;
const           s64             s_spam_maxSize      = _4gb * 4;     // a generous 16gb!

static xMutex           s_mtx_unilogger;
static FILE*            s_myLog             = nullptr;
static s64              s_myLog_Written     = 0;

void LogHostInit()
{
    s_mtx_unilogger.Create();
}

static void _debugBreakContext( xString& context, const AssertContextInfoTriad& triad )
{
    const char* threadname = thread_getname();

    context.Format(
        "Condition  :  %s\n"
        "Function   :  %s\n"
        "Thread     :  %s\n",

        triad.cond ? triad.cond : "(none)",
        triad.funcname,
        threadname ? threadname : "(unknown)"
    );
}

static void _flush_all_that_filesystem_jazz()
{
    fflush(nullptr);
}

// --------------------------------------------------------------------------------------
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

    const char* title = "Application Error";

    switch (breakType) {
        case DbgBreakType_Assert:   title = "Assertion Failure";        break;
        case DbgBreakType_Abort:    title = "Application Error";        break;

        default:
            xPrintLn(xFmtStr("Unknown DbgBreakType Type = %d will be treated as abort (Application Error)", breakType));
            title = "Application Error";
        break;
    }

    xPrintLn( xFmtStr("%s%s\n\nContext:\n%s", triad.filepos, message.c_str(), context.c_str()) );
    _flush_all_that_filesystem_jazz();

    assert_t breakit = assert_none;

    breakit = Host_AssertionDialog( title, message, context );

    if (breakit == assert_terminate_app)
    {
        // this will no doubt fail miserably.  Need a custom exit() function that calls various
        // thread shutdown destructors in the appropriate order.

        exit(-1);
    }

    return breakit;
}

// --------------------------------------------------------------------------------------
assert_t xDebugBreak( DbgBreakType breakType, const AssertContextInfoTriad& triad, const char* fmt, ... )
{
    va_list list;
    va_start(list, fmt);
    assert_t result = xDebugBreak_v( breakType, triad, fmt, list );
    va_end(list);

    return result;
}


static void _openNewLogFile()
{
    todo("oops!  logfile not implemented.");
}

static void advanceMyLog( int numChars )
{
    s_myLog_Written += numChars;
    if (s_myLog_Written > s_mylog_MaxFileSize)
    {
        _openNewLogFile();
    }
}

// --------------------------------------------------------------------------------------
void xPrintLn(const xString& msg)
{
    xScopedMutex lock(s_mtx_unilogger);

    if (1)                  { xOutputString(msg + "\n", stdout);    }
    if (s_myLog)            { fputs(msg + "\n", s_myLog);           }
}

static void _host_log_v(FILE* std_fp, const char* fmt, va_list list)
{
    if (!fmt || !fmt[0])
    {
        // just treat it as a newline, no prefixing or other mess.
        // (sometimes used to generate visual separation in the console output)

        xScopedMutex lock(s_mtx_unilogger);
        xOutputString("\n", std_fp);
        if (s_myLog) fputs("\n", s_myLog);
        return;
    }

    xString buffer;

    if (fmt && fmt[0])
    {
        buffer.AppendFmtV(fmt, list);
        buffer += "\n";
    }

    xScopedMutex lock(s_mtx_unilogger);

    xOutputString(buffer, std_fp);

    if (s_myLog)
    {
        fputs(buffer, s_myLog);
        //fflush(s_myLog);      // fflush(nullptr) performed on sigsegv handler.
        advanceMyLog(buffer.GetLength() + 10);
    }

    //spamAbortCheck(buffer.GetLength() + 10);
}

void log_host(const char* fmt, ...)
{
    va_list list;
    va_start(list, fmt);
    _host_log_v(stdout, fmt, list);
    va_end(list);
}

void log_host_v(const char* fmt, va_list list)
{
    _host_log_v(stdout, fmt, list);
}

void warn_host(const char* fmt, ...)
{
    va_list list;
    va_start(list, fmt);
    _host_log_v(stderr, fmt, list);
    va_end(list);
}

void warn_host_v(const char* fmt, va_list list)
{
    _host_log_v(stderr, fmt, list);
}


void flush_log()
{
    xScopedMutex lock(s_mtx_unilogger);
    if(s_myLog) {
        fflush(s_myLog);
    }
}
