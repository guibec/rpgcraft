
#pragma once

#include "x-types.h"
#include <cstdarg>
#include <cstdlib>		// needed for abort()

// --------------------------------------------------------------------------------------
#if defined(__clang__)
#	define __stop()		__builtin_trap()
#else
#	define __stop()		xStopProcess( __FILEPOS__, __FUNCTION_NAME__ )
#endif
// --------------------------------------------------------------------------------------


// --------------------------------------------------------------------------------------
// __stop_final() for use where process continuation may be ideal in a testing environment, in
// spite of serious error.  This is the default behavior for log_abort(), where it is left to
// the descretion of the developer whether to honor or ignore various types of errors (such as
// snapshot and thread deadlock errors).
//
#if TARGET_RELEASE
#	define __stop_final()		__builtin_trap()
#else
#	define __stop_final()		(void(0))
#endif
// --------------------------------------------------------------------------------------


// --------------------------------------------------------------------------------------
//                              Static Compilation Options
// --------------------------------------------------------------------------------------

#if defined(ID_jstine)
// mine overrides!
//#	define DISABLE_ASSERTIONS	1
//#	define ENABLE_QA_ASSERT		0
#endif

// --------------------------------------------------------------------------------------
// DISABLE_ASSERTIONS
//
// Force-disables all assertions.  Defaults to 1 in FINAL builds, and can be set to 1 for
// debug/devel build to disable assertions there as well.  (for what reason, who knows)
//
#if !defined(DISABLE_ASSERTIONS)
#	if TARGET_RELEASE
#		define DISABLE_ASSERTIONS		1
#	else
#		define DISABLE_ASSERTIONS		0
#	endif
#endif
// --------------------------------------------------------------------------------------


// --------------------------------------------------------------------------------------
// ENABLE_DEBUG_ASSERT
//
// Enables standard debug assertions.  Typically linked to TARGET_DEBUG but can be
// overridden at compile time if debug assertions are desired as part of a QA build.
//    (this option will be disregarded if DISABLE_ASSERTIONS == 1)
//
#if !defined(ENABLE_DEBUG_ASSERT)
#	define ENABLE_DEBUG_ASSERT			TARGET_DEBUG
#endif
// --------------------------------------------------------------------------------------

// --------------------------------------------------------------------------------------
// ENABLE_QA_ASSERT
//
// Enables the bug_qa and bug_on_qa assertion macro in optimized (release target) builds.
// Since bug_opt checks are only used in areas where performance is not an issue,
// this option is enabled by default and is safe to leave enabled for all builds.
//    (this option will be disregarded if DISABLE_ASSERTIONS == 1)
//
#if !defined(ENABLE_QA_ASSERT)
#	if TARGET_DEBUG || TARGET_QA
#		define ENABLE_QA_ASSERT			1
#	else
#		define ENABLE_QA_ASSERT			0
#	endif
#endif
// --------------------------------------------------------------------------------------


// --------------------------------------------------------------------------------------
// ENABLE_DEVEL_LOG / ENABLE_QA_LOG / ENABLE_BENCH_LOG / ENABLE_ABORT_LOG
//
// Enables inclusion development-useful logging information, typically denoted through the
// use of the log_devel function/macro.  Development logs include things that are either not
// needed in a production-server emulator environment (where excess logging could impact
// performance) or information that could be considered a security risk.
//
#if !defined(ENABLE_DEVEL_LOG)
#	if	TARGET_DEBUG || ISD_TOOLING_BUILD
#		define ENABLE_DEVEL_LOG			1
#	else
#		define ENABLE_DEVEL_LOG			0
#	endif
#endif

#if !defined(ENABLE_QA_LOG)
#	if	TARGET_DEBUG || TARGET_QA
#		define ENABLE_QA_LOG			1
#	else
#		define ENABLE_QA_LOG			0
#	endif
#endif

#if !defined(ENABLE_BENCH_LOG)
#	if	TARGET_RELEASE
#		define ENABLE_BENCH_LOG			0
#	else
#		define ENABLE_BENCH_LOG			1
#	endif
#endif

#if !defined(ENABLE_PERF_LOG)
#	if	TARGET_RELEASE
#		define ENABLE_PERF_LOG			0
#	else
#		define ENABLE_PERF_LOG			1
#	endif
#endif

#if !defined(ENABLE_ABORT_LOG)
#	define ENABLE_ABORT_LOG				1
#endif
// --------------------------------------------------------------------------------------

// --------------------------------------------------------------------------------------
// ENABLE_POPUP_ERRORS
//
// Allows popup dialogs for development assertions to be displayed to the host, if the host
// supports popup windows.  On hosts lacking popup windows, popup requests will be logged to
// disk and execution will proceed unimpeeded.
//
// Only effective if ENABLE_ABORT_LOG=1
//
#if !defined(ENABLE_POPUP_ERRORS)
#	if TARGET_RELEASE
#		define ENABLE_POPUP_ERRORS			0
#	else
#		define ENABLE_POPUP_ERRORS			1
#	endif
#endif
// --------------------------------------------------------------------------------------

// --------------------------------------------------------------------------------------
// ENABLE_WARNING_LOGS
//
// Used to filter in/out warn() and warn_on() macro output.
//
#if !defined(ENABLE_WARNING_LOGS)
#	if TARGET_RELEASE || TARGET_BENCHMARK
#		define ENABLE_WARNING_LOGS		0
#	else
#		define ENABLE_WARNING_LOGS		1
#	endif
#endif
// --------------------------------------------------------------------------------------

// --------------------------------------------------------------------------------------
// ENABLE_HOST_TRACE_LOGS
//
#if !defined(ENABLE_HOST_TRACE_LOGS)
#	if TARGET_RELEASE || TARGET_BENCHMARK
#		define ENABLE_HOST_TRACE_LOGS	0
#	else
#		define ENABLE_HOST_TRACE_LOGS	1
#	endif
#endif
// --------------------------------------------------------------------------------------


// --------------------------------------------------------------------------------------
//                               Host Logging Facilities
// --------------------------------------------------------------------------------------
//  volume_log       - directs to logfile only (bypasses stdout and debug console)
//  verbose_log      - directs to stdout, debug console, and logfile.
//  xDebugBreak		 - generates a verbose log and then causes a debug break.
//
//  These logs are always functional; no combination of compiler options disables
//  them.  xDebugBreak can even be used during optimized release target builds.
//  Target-conditional logs are controlled through the use of macros.
//
//  Note that the debug console may only be available (or implemented) on windows.
//

enum assert_t {
	assert_ignore_all    = -1,
	assert_none          =  0,
	assert_ignore_once   =  0,
	assert_break         =  1,
	assert_terminate_app = 2
};

enum DbgBreakType
{
	// standard host debug assertion dialog
	DbgBreakType_Assert,

	// standard non-debug abortion
	DbgBreakType_Abort,
};

enum xLogFlag {
	xLogFlag_Default		= 0,
	xLogFlag_Important		= (1 << 0),		// warnings or errors, honored by log_verbose only, also assumes NoRepeat.
	xLogFlag_Undecorated	= (1 << 1),		// dnoes not print any timestamp information  (for lua/compiler errors)
};

extern void		_host_log			(uint flags, const char* moduleName, const char* fmt = nullptr, ...)	__verify_fmt(4, 5);
extern void		_host_log_v			(uint flags, const char* moduleName, const char* fmt, va_list params);

extern void		xPrintLn			(const xString& msg);
extern void		flush_log			();

extern void		xLogSetMaxSpamSize	(s64 newSize);

extern void		xStopProcess		( const char* filepos, const char* funcname );

extern assert_t	xDebugBreak			( DbgBreakType breakType, const char* filepos, const char* funcname, const char* cond, const char* fmt=nullptr, ... ) __verify_fmt(5, 6);
extern assert_t xDebugBreak_v		( DbgBreakType breakType, const char* filepos, const char* funcname, const char* cond, const char* fmt, va_list list );

// Writes directly to debug console where supported (visual studio), or stdout otherwise.
extern void		xOutputString		(const char* str);

// Writes directly to debug console where supported (visual studio), or stderr otherwise.
extern void		xOutputStringError(const char* msg);

extern bool		xIsDebuggerAttached	();

#define log_host(...)						 _host_log		(xLogFlag_Default,		s_ModuleName,			## __VA_ARGS__)
#define log_host_loud(...)					 _host_log		(xLogFlag_Important,	s_ModuleName,			## __VA_ARGS__)
#define warn_host(...)						 _host_log		(xLogFlag_Important,	s_ModuleName, "WARN: "	## __VA_ARGS__)
#define warn_host_on(cond,...)	((cond) &&	(_host_log		(xLogFlag_Important,	s_ModuleName, "WARN: "	## __VA_ARGS__), true))

#if ENABLE_DEBUG_LOG
#	define log_debug(fmt, ...)			_host_log( xLogFlag_Default, s_ModuleName, fmt, ## __VA_ARGS__ )
#else
#	define log_debug(fmt, ...)			((void)0)
#endif

#if ENABLE_QA_LOG
#	define log_qa(fmt, ...)				_host_log( xLogFlag_Default, s_ModuleName, fmt, ## __VA_ARGS__ )
#else
#	define log_qa(fmt, ...)				((void)0)
#endif

#if ENABLE_PERF_LOG
#	define log_perf( fmt, ... )			_host_log( xLogFlag_Default, s_ModuleName, fmt, ## __VA_ARGS__ )
#else
#	define log_perf( fmt, ... )			((void)0)
#endif

#if ENABLE_ABORT_LOG
#	if ENABLE_POPUP_ERRORS && !DISABLE_ASSERTIONS
#		define	log_and_abort(...)								 (void(_inline_abort_("error/abort", ## __VA_ARGS__)))
#		define	log_and_abort_on(cond,...)			( (cond) &&  (    (_inline_abort_( # cond,       ## __VA_ARGS__)), true) )
#	else
#		define	log_and_abort(...)					             (_host_log(xLogFlag_Important, "abort",	## __VA_ARGS__),	__stop())
#		define	log_and_abort_on(cond,...)			( (cond) &&  (_host_log(xLogFlag_Important, "abort",	## __VA_ARGS__),	__stop(), true) )
#	endif
#	define		abort_var(...)						__VA_ARGS__
#else
#	define		log_and_abort(...)					(			 (__stop())
#	define		log_and_abort_on(cond,...)			( (cond) &&  (__stop(), true) )
#	define		abort_var(...)
#endif

// --------------------------------------------------------------------------------------
//              HostTraceEnter / HostTraceLeave / HostTraceCheckpoint
// --------------------------------------------------------------------------------------
// HostTraceCheckpoint
//    Intended for development trace logging in situations where a debugger is not available.
//    Spam this on every other line of code around a place that's crashing to get an idea
//    where things go bad.  (src code line number is logged in each case).
//
#if ENABLE_HOST_TRACE_LOGS
#	define HostTraceEnter()			_verbose_log( 0, __FILEPOS__, __FUNCTION_NAME__, " entered." )
#	define HostTraceLeave()			_verbose_log( 0, __FILEPOS__, __FUNCTION_NAME__, " completed." )
#	define HostTraceCheckpoint()	_verbose_log( 0, __FILEPOS__, __FUNCTION_NAME__, " --> Checkpoint Reached" )
#else
#	define HostTraceEnter()			((void)0)
#	define HostTraceLeave()			((void)0)
#	define HostTraceCheckpoint()	((void)0)
#endif
// --------------------------------------------------------------------------------------


// --------------------------------------------------------------------------------------
//                                 Assertion Macros
// --------------------------------------------------------------------------------------
// Usage:
//   bug_on
//      thread-safe assertion active in debug builds only.  Assertion is logged to
//      disk and to debug output (if each is enabled).
//
//   bug_on_qa
//      thread-safe assertion active in debug AND optimized release builds.  Due to a
//      lack of debug/trace info in optimized builds, these types of assertions should
//      always provide a user-readable text message.
//
//   assume
//		Directs the compiler/optimizer to make the given assumption for Release (optimized)
//      build targets.  Debug builds will generate an assertion if the condition fails.
//      For unreachable code scenarios, see UNREACHABLE(msg)
//
//   unreachable
//      Tells the compiler/optimizer that the following code path is unreachable.  This
//      assumption is made for Release and Final builds.  Debug builds will generate
//      an assertion if the condition fails.
//
//   unreachable_qa
//      Tells the compiler/optimizer that the following code path is unreachable.  This
//      assumption is only made for Final builds.  Debug and Release builds will generate
//      an assertion if the condition fails.
//

//
// Rationale:
//   The assertion tools currently provided by both Microsoft Windows and Linux platforms
//   are woefully inadequate for handling assertions within the context of a multithreaded
//   application.  Microsoft's can cause race conditions and assertions on threads other
//   than the main thread are sometimes missed (as of Windows SDK v7).  Linux assertions are
//   nothing more than __debugbreak() calls that require GDB or something else to make them
//   useful.
//
//   Assumption and unreachable tools are a new addition to compilers, made available in
//   Visual Studio .NET in 2002 and now also available in GCC v4.5+.  The most common uses
//   for optimization assumptions are switch statements (unreachable 'default:' case) and
//   for(;;) loops where the loop count is known to be within strict parameters.
//
// Implementation:
//   All assertion macros use block-safe syntax, such that safeguard curly braces {} should
//   not be required.  This is achieved through use of C's comma (,) operator.
//
//   Some of the macros use the C99 standard form of variadic parameters via __VA_ARGS__.
//   This may not be supported on older versions of GCC (prior to 4.2?).
//

#undef ASSERT
#undef ASSERT_QA
#undef ASSERT_ALL

#undef ASSUME

// -------------------------------------------------------------------------------------
#if !defined(ALLOW_IGNORABLE_ASSERT)
#	if TARGET_ORBIS || TARGET_RELEASE
#		define ALLOW_IGNORABLE_ASSERT		0
#	else
#		define ALLOW_IGNORABLE_ASSERT		1
#	endif
#endif
// -------------------------------------------------------------------------------------


#if DISABLE_ASSERTIONS
#	define _inline_debugbreak_(prefix,...)		(false)
#	define _inline_abort_(prefix,...)			(false)

#else

// This enables support for the "Ignore All" button in the assertion dialog, so you can
// continue the game w/o being spammed with the same assertion again...
// It works by creating unique DebugBreak implementations via __COUNTER__ and template
// functions.  Functions are assured unique function-scope static vars via anonymous
// namespace encapsulation.
//
// Thread safety Caveat:
//  * Clicking "Ignore All" on an assertion while another thread is generating the very
//    same assertion may not immediately take effect, since the assertion recursion check
//    occurs after the Ignore-All check.  Not worth fixing, we assume...

#define EXPAND( x ) x

namespace
{
	template< int hash_key > assert_t _Ignorable_DebugBreak(DbgBreakType breakType, const char* filepos, const char* funcname, const char* cond, const char* fmt=nullptr, ... ) __verify_fmt(5, 6);
	template< int hash_key > assert_t _Ignorable_DebugBreak(DbgBreakType breakType, const char* filepos, const char* funcname, const char* cond, const char* fmt, ... ) {
		#if ALLOW_IGNORABLE_ASSERT
		static int		done	= 0;
		if (done) return assert_none;
		#endif

		va_list list;
		va_start(list, fmt);
		assert_t result = xDebugBreak_v( breakType, filepos, funcname, cond, fmt, list);
		va_end(list);

		#if !TARGET_ORBIS
		if (result == assert_ignore_all) {
			done = 1;
		}
		#endif

		return result;
	}
}

#	define _inline_debugbreak_(prefix,...)		((_Ignorable_DebugBreak<__COUNTER__>(DbgBreakType_Assert,	__FILEPOS__, __FUNCTION_NAME__, prefix , ##__VA_ARGS__) == assert_break) && (__debugbreak(), true))
#	define _inline_abort_(prefix,...)			((_Ignorable_DebugBreak<__COUNTER__>(DbgBreakType_Abort,	__FILEPOS__, __FUNCTION_NAME__, prefix , ##__VA_ARGS__) == assert_break) && (__debugbreak(), true))

#endif

// Assertion macros active in all builds regardless of compiler options/optimizations.
// These macros are primarily for internal use only (other conditional build assertions
// are constructed from them).

#define unreachable_all(...)			((void)(_inline_debugbreak_( "unreachable" , ## __VA_ARGS__), __unreachable()))

#if TARGET_DEBUG
#	define unreachable(...)				unreachable_all		( __VA_ARGS__ )
#	define assume(cond, ...)			(!(cond) && (_inline_debugbreak_("(assume)" # cond , ## __VA_ARGS__), false))
#else
#	define unreachable(...)				__unreachable()
#	if defined(__clang__)
#		define assume(cond, ...)		(!(cond) && (__builtin_unreachable(),false))
#	elif GCC_CHECK_VER( 4, 5 )
#		define assume(cond, ...)		(!(cond) && (__builtin_unreachable(),false))
#	elif defined(_MSC_VER)
#		define assume(cond, ...)		__assume( cond )
#	else
#		define assume(cond, ...)		((void)0)
#	endif

#endif

#if ENABLE_QA_ASSERT || TARGET_DEBUG
#	define assume_qa(cond, ...)			(!(cond) && (_inline_debugbreak_("(assume)" # cond , ## __VA_ARGS__), false))
#	define unreachable_qa(...)			unreachable_all		( __VA_ARGS__ )
#else
#	define assume_qa(cond, ...)			assume(cond)
#	define unreachable_qa(...)			__unreachable()
#endif

// -------------------------------------------------------------
//                   warn / todo / bug macros
// -------------------------------------------------------------
// Unlike assertions, these macros execute if the condition is true, not false.  These are
// typically more intuitive in most cases.  These macros can be used in the same syntax
// as ASSERT macros above, such as part of if() statements.
//
// Note that use of 'bug()' (without a condition) should only be used in situations where
// use of 'unreachable()' is undesirable for some reason, such as wanting to retain "best guess"
// behavior even in final builds.
//
#if ENABLE_WARNING_LOGS
#	define	warn(msg, ...)							_host_log( xLogFlag_Important, s_ModuleName, "WARN: " msg,			## __VA_ARGS__ )
#	define	warn_on(cond, ...)	 	( (cond) && (	_host_log( xLogFlag_Important, s_ModuleName, "WARN: " # cond ", " 	## __VA_ARGS__ ), true) )
#else
#	define	warn(...) 				(false)
#	define	warn_on(cond, ...)	 	(false)
#endif


#define	todo_all(...)							 _inline_debugbreak_("!TODO!",				## __VA_ARGS__)
#define	bug_all(...)							 _inline_debugbreak_("(bug)",				## __VA_ARGS__)
#define	bug_on_all(cond, ...)		( (cond) && (_inline_debugbreak_("(bug_on) " # cond,	## __VA_ARGS__),	true) )

#if ENABLE_DEBUG_ASSERT
#	define	todo(...)					todo_all			(			__VA_ARGS__ )
#	define	bug(...)					bug_all				(			__VA_ARGS__ )
#	define	bug_on(cond, ...)			bug_on_all			( cond,  ## __VA_ARGS__ )
#	define	bug_var(...)				__VA_ARGS__
#else
#	if TARGET_RELEASE
#		define	todo(...)				__unreachable()
#	else
#		define	todo(...)				todo_all( __VA_ARGS__ )
#	endif
#	define	bug(...)					(false)
#	define	bug_on(cond, ...)			(false)
#	define	bug_var(...)
#endif

#if ENABLE_QA_ASSERT
#	define	bug_qa(...)					bug_all				(			__VA_ARGS__ )
#	define	bug_on_qa(cond, ...)		bug_on_all			( cond,	 ## __VA_ARGS__ )
#	define	bug_var_qa(...)				__VA_ARGS__
#else
#	define	bug_qa(...)					(false)
#	define	bug_on_qa(cond, ...)		(false)
#	define	bug_var_qa(...)
#endif

extern assert_t	Host_AssertionDialog		(const xString& title, const xString& message, const xString& context);

// --------------------------------------------------------------------------------------
inline __ai void xStopProcess(const char* filepos, const char* func)
{
	// do not call log_abort() here, causes recursion...
	if (ENABLE_ABORT_LOG) {
		_host_log( xLogFlag_Important, nullptr, filepos, "(function '%s'): Process halted by way of __stop() or similar.", func );
	}

	abort();
}
