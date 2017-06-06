#include "x-types.h"
#include "x-string.h"
#include "x-thread.h"
#include "x-assertion.h"

#include <chrono>


// s_mylog_MaxFileSize
//  Above this length, the current logfile is closed and a new one is opened for continue log
//  generation.  Some editors can handle larger files (1-2gb is commonl allowable) but they
//  tend to handle those sizes poorly -- especially when doing diffs.  So splitting them up
//  usually is a low-hassle way to speed up viewing/diffing. --jstine
//
const			s64				s_mylog_MaxFileSize	= _256mb;
const			s64				s_spam_maxSize		= _4gb * 4;		// a generous 16gb!

static xMutex	s_mtx_unilogger;
static FILE*	s_myLog				= nullptr;
static s64		s_myLog_Written		= 0;

static std::chrono::steady_clock::time_point s_HostLogTimer;

void LogHostInit()
{
	s_mtx_unilogger.Create();
}

static void _debugBreakContext( xString& context, const char* filepos, const char* funcname, const char* cond )
{
	const char* threadname = thread_getname();

	context.Format(
		"Condition  :  %s\n"
		"Function   :  %s\n"
		"Thread     :  %s\n",

		cond ? cond : "(none)",
		funcname,
		threadname ? threadname : "(unknown)"
	);
}

static void _flush_all_that_filesystem_jazz()
{
	fflush(nullptr);
}

// --------------------------------------------------------------------------------------
assert_t xDebugBreak_v( DbgBreakType breakType, const char* filepos, const char* funcname, const char* cond, const char* fmt, va_list list )
{
	static __threadlocal int s_recursion_guard = 0;

	xScopedIncremental	guard( s_recursion_guard );

	if (!guard.TryEnter())
	{
		__debugbreak();
		exit(66);
	}

	xString context;
	_debugBreakContext( context, filepos, funcname, cond );

	xString message;
	if (fmt) {
		message.FormatV(fmt, list);
	}

	const char* title = "Application Error";

	switch (breakType) {
		case DbgBreakType_Assert:	title = "Assertion Failure";		break;
		case DbgBreakType_Abort:	title = "Application Error";		break;

		default:
			xPrintLn_loud(xFmtStr("Unknown DbgBreakType Type = %d will be treated as abort (Application Error)", breakType));
			title = "Application Error";
		break;
	}

	xPrintLn_loud( xFmtStr("%s: *** ASSERTION FAILURE ***\n%s\n\nContext:\n%s", filepos, message.c_str(), context.c_str()) );
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
assert_t xDebugBreak( DbgBreakType breakType, const char* filepos, const char* funcname, const char* cond, const char* fmt, ... )
{
	va_list list;
	va_start(list, fmt);
	assert_t result = xDebugBreak_v( breakType, filepos, funcname, cond, fmt, list );
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
static void vlog_append_host_clock(xString& dest)
{
	xScopedMutex lock(s_mtx_unilogger);

	auto newtime = std::chrono::steady_clock::now();

	lock.Unlock();

	typedef std::chrono::duration<double> seconds_d;
	auto secs = std::chrono::duration_cast<seconds_d>(newtime - s_HostLogTimer);
	dest.AppendFmt("[%8.03fsec] ", secs);
}

// --------------------------------------------------------------------------------------
template<bool isImportant>
void xPrintLn_impl(const xString& msg)
{
	xScopedMutex lock(s_mtx_unilogger);

	//if(vlog_check_history(LogType_Verbose, NULL, msg))
	//	return;

	xString buffer = "        ";
	vlog_append_host_clock(buffer);
	buffer += msg;
	buffer += "\n";

	if (isImportant)		{ xOutputVerboseString(buffer); }
	else					{ xOutputDebugString  (buffer); }
	if (s_myLog)			{ fputs(buffer, s_myLog);		}
}


void xPrintLn		(const xString& msg) { xPrintLn_impl<false>(msg); }
void xPrintLn_loud	(const xString& msg) { xPrintLn_impl<true >(msg); }

// --------------------------------------------------------------------------------------
void _host_log(uint flags, const char* moduleName, const char* fmt, ...)
{
	if (!fmt || !fmt[0])
	{
		// just treat it as a newline, no prefixing or other mess.
		// (sometimes used to generate visual separation in the console output)

		xScopedMutex lock(s_mtx_unilogger);
		xOutputDebugString("\n");
		if (s_myLog) fputs("\n", s_myLog);
		return;
	}

	xString buffer;
	if (moduleName && moduleName[0])
		buffer.Format("%-8s", moduleName);
	else
		buffer = "        ";

	vlog_append_host_clock(buffer);

	if (fmt && fmt[0])
	{
		va_list list;
		va_start(list, fmt);
		buffer.AppendFmtV(fmt, list);
		buffer += "\n";
		va_end(list);
	}

	xScopedMutex lock(s_mtx_unilogger);

	if (flags & xLogFlag_Important) { xOutputVerboseString(buffer); }
	else							{ xOutputDebugString  (buffer); }

	if (s_myLog)
	{
		fputs(buffer, s_myLog);
		//fflush(s_myLog);		// fflush(nullptr) performend on sigsegv handler.
		advanceMyLog(buffer.GetLength() + 10);
	}

	//spamAbortCheck(buffer.GetLength() + 10);
}

void _host_log_v(uint flags, const char* moduleName, const char* fmt, va_list list)
{
	if (!fmt || !fmt[0])
	{
		// just treat it as a newline, no prefixing or other mess.
		// (sometimes used to generate visual separation in the console output)

		xScopedMutex lock(s_mtx_unilogger);
		xOutputDebugString("\n");
		if (s_myLog) fputs("\n", s_myLog);
		return;
	}

	xString buffer;
	if (moduleName && moduleName[0])
		buffer.Format("%-8s", moduleName);
	else
		buffer = "        ";

	vlog_append_host_clock(buffer);

	if (fmt && fmt[0])
	{
		buffer.AppendFmtV(fmt, list);
		buffer += "\n";
	}

	xScopedMutex lock(s_mtx_unilogger);

	if (flags & xLogFlag_Important) { xOutputVerboseString(buffer); }
	else							{ xOutputDebugString  (buffer); }

	if (s_myLog)
	{
		fputs(buffer, s_myLog);
		//fflush(s_myLog);		// fflush(nullptr) performend on sigsegv handler.
		advanceMyLog(buffer.GetLength() + 10);
	}

	//spamAbortCheck(buffer.GetLength() + 10);
}

void flush_log()
{
	xScopedMutex lock(s_mtx_unilogger);
	if(s_myLog) {
		fflush(s_myLog);
	}
}
