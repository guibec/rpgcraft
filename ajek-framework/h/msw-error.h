
#pragma once

#include "x-types.h"

// Logs error message info as provided by the Windows API.
extern void msw_LogLastError( const char* filepos, const char* funcname, int lastError, const char* fmt=nullptr, ...  );

// Logs error message info as provided by the Windows API, and generates an
// assertion (debugbreak).  Assertion happens in all build targets!
extern bool msw_AssertLastError( const char* filepos, const char* funcname, int lastError, const char* fmt=nullptr, ... );

#if DISABLE_ASSERTIONS
#	define msw_bug_any(...)					((void)0)
#	define msw_assert_any( cond, ... )		((void)0)
#	define msw_bug_on_any( cond, ... )		((void)0)
#else
#	define msw_bug_any(...)					((void)( msw_AssertLastError( __FILEPOS__, __FUNCTION_NAME__, ::GetLastError(), ## __VA_ARGS__ ) && (__debugbreak(), false) ))
#	define msw_assert_any( cond, ... )		( !(cond) && (msw_bug_any(__VA_ARGS__), false) )
#	define msw_bug_on_any( cond, ... )		(  (cond) && (msw_bug_any(__VA_ARGS__), true) )
#endif

#if TARGET_DEBUG
#	define msw_assert( cond, ... )			msw_assert_any	( cond, ## __VA_ARGS__ )
#	define msw_bug_on( cond, ... )			msw_bug_on_any	( cond, ## __VA_ARGS__ )
#	define msw_bug(...)						msw_bug_any		(__VA_ARGS__)
#else
#	define msw_assert( cond, ... )			(false)
#	define msw_bug_on( cond, ... )			(false)
#	define msw_bug(...)						((void)0)
#endif

#if !TARGET_FINAL
#	define msw_assert_qa( cond, ... )		msw_assert_any	( cond, ## __VA_ARGS__ )
#	define msw_bug_on_qa( cond, ... )		msw_bug_on_any	( cond, ## __VA_ARGS__ )
#	define msw_bug_qa(...)					msw_bug_any		(__VA_ARGS__)
#	define msw_bug_on_log( cond, ...)		msw_bug_on_any	( cond, ## __VA_ARGS__ )
#	define msw_bug_log(...)					msw_bug_any		(__VA_ARGS__)
#else
#	define msw_assert_qa( cond, ... )		(false)
#	define msw_bug_on_qa( cond, ... )		(false)
#	define msw_bug_qa(...)					((void)0)
#	define msw_bug_on_log( cond, ...)		( (cond) || (log_host( __VA_ARGS__ ), true) )
#	define msw_bug_log(...)					( host_log( __VA_ARGS__ ) )
#endif

