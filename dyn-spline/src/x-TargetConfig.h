
#pragma once

// ======================================================================================
//  x-TargetConfig.h : Olympus Target (Host) Platform Configuration
// ======================================================================================


// --------------------------------------------------------------------------------------
// TARGET_x64 / TARGET_x32
//
// Mutually exclusive indicators for 32 or 64 bit intel architectures.  Intended for
// use with code which is 32/64 bit dependant but not operating system dependent.
//
#if defined(_MSC_VER)
#	if defined(_M_X64) || defined(_M_AMD64) || defined(_WIN64)
#		define TARGET_x64	1
#		define TARGET_x32	0
#		undef _M_IX86		// why is it insisting on being defined by the compiler?
#	elif defined(_M_IX86)
#		define TARGET_x64	0
#		define TARGET_x32	1
#	endif

#elif defined(__ORBIS__)
#		define TARGET_x64	1
#		define TARGET_x32	0

#elif	defined(__GNUC__)
#	if defined(__amd64__) || defined(__x86_64__)
#		define TARGET_x64	1
#		define TARGET_x32	0
#	elif defined(__i386__)
#		define TARGET_x64	0
#		define TARGET_x32	1
#	endif
#endif

// --------------------------------------------------------------------------------------
// TARGET_ORBIS
//
#if !defined(TARGET_ORBIS)
#	if defined(__ORBIS__)
#		define TARGET_ORBIS		1
#	else
#		define TARGET_ORBIS		0
#	endif
#endif
// --------------------------------------------------------------------------------------

// --------------------------------------------------------------------------------------
// TARGET_MSW
//
// More adeptly-named Replacement for _WIN32 (which is oddly defined for both 32 and
// 64 bit varieties of Windows)
//
#if !defined(TARGET_MSW)
#	if defined(_WIN32) || defined(_WIN64)
#		define TARGET_MSW	1
#	else
#		define TARGET_MSW	0
#	endif
#endif
// --------------------------------------------------------------------------------------

// --------------------------------------------------------------------------------------
// TARGET_LINUX
//
// Set to 1 when compiling for LINUX operating systems (Ubuntu, etc).  No autodetection is
// performed since there's hardly any standard define present to identify linux systems
//
#if !defined(TARGET_LINUX)
#	define TARGET_LINUX		0
#endif
// --------------------------------------------------------------------------------------

// --------------------------------------------------------------------------------------
// TARGET_WIN64
//
// Indicates the presence of a 64-bit Windows operating system.  Some Windows features
// such as SEH and the ABI differ greatly between 32 and 64 bit varieties.
//
#if !defined(TARGET_WIN64)
#	if TARGET_MSW && TARGET_x64
#		define TARGET_WIN64	1
#	else
#		define TARGET_WIN64	0
#	endif
#endif
// --------------------------------------------------------------------------------------


#if !defined(TARGET_LINUX) && !defined(TARGET_MSW) && !defined(TARGET_ORBIS)
#	error Unsupported or undefined target platform.
#endif


// ======================================================================================
//  Debug, Benchmark, QA, and Final Target configurations

#if !defined(TARGET_DEBUG)
#	define TARGET_DEBUG		0
#endif

#if !defined(TARGET_QA)
#	define TARGET_QA		0
#endif

#if !defined(TARGET_RELEASE)
#	define TARGET_RELEASE	0
#endif

#if !TARGET_DEBUG && !TARGET_QA && !TARGET_RELEASE
#	error A valid target type has not been set.  TARGET_DEBUG, TARGET_BENCHMARK, TARGET_QA, and TARGET_FINAL are all zero (0).
#endif
// ======================================================================================

// ======================================================================================
//  ABI Selector  (Advanced Binary Interface)
// ======================================================================================
// Supported are SystemV (BSD/Linux/Mac) and Win64 ABIs.
// Quick summary of the relevant differences:
//   SystemV : Passes params on regs rdi, rsi, rdx, rcx, r8, r9
//           : RSI, RDI, and all XMMs are scratch.
//   Win64   : Passes params on regs rcd, rdx, r8, r9
//           : RSI, RDI, and XMMs 6-15 are preserved (callee-saved).
//

#if !defined(ABI_SYSTEMV) && !defined(ABI_WIN64)
#	if TARGET_MSW
#		define ABI_SYSTEMV			0
#	else
#		define ABI_SYSTEMV			1
#	endif
#endif

#if !defined(ABI_WIN64)
#	if ABI_SYSTEMV
#		define ABI_WIN64			0
#	else
#		define ABI_WIN64			1
#	endif
#else
	// engineering has specified ABI_WIN64={0,1} on the makefile.  Honor it by also
	// mirroring !value to ABI_SYSTEMV if it hasn't already been defined.
#	if !defined(ABI_SYSTEMV)
#		if ABI_WIN64
#			define ABI_SYSTEMV		0
#		else
#			define ABI_SYSTEMV		1
#		endif
#	endif
#endif

#if !ABI_WIN64 && !ABI_SYSTEMV
#	error A valid ABI has not been set.  TARGET_SYSTEMV and TARGET_WIN64 are both zero (0).
#endif

#if ABI_WIN64 && ABI_SYSTEMV
#	error A valid ABI has not been set.  TARGET_SYSTEMV and TARGET_WIN64 are both one (1).
#endif

// ======================================================================================
//  Debugging Features
// ======================================================================================

// --------------------------------------------------------------------------------------
// ENABLE_DEBUG_OUTPUT
//
// (MSW/MSVC only) Enables use of the debugger's "output" window for warn and devel_log statements.
//
#if !defined(MSW_ENABLE_DEBUG_OUTPUT)
#	if TARGET_FINAL
#		define MSW_ENABLE_DEBUG_OUTPUT		0
#	else
#		define MSW_ENABLE_DEBUG_OUTPUT		1
#	endif
#endif
// --------------------------------------------------------------------------------------

// --------------------------------------------------------------------------------------
// USE_SMALLER_TYPE_CHECKS
//
// Set this to 1 when using smaller type checking (MSDN /RTCc)
// When enabled, several int->char and int->word conversions will use explicit bit masks
// instead of typecasts alone.  This disables the MSVCRT's assertion throws when enabling
// compiler type checking support.
//
#if !defined(USE_SMALLER_TYPE_CHECKS)
#	define USE_SMALLER_TYPE_CHECKS			0
#endif
// --------------------------------------------------------------------------------------

// --------------------------------------------------------------------------------------
// USE_GLIBC_MACRO_FIXUP
//
// Set this to 1 to fix compilation errors caused by our completely valid and justifiable
// use of __unused and __xi macros.  Should only need set to 1 when compiling using authentic
// Linux/glibc headers.
//
// Why not just use __GNU_LIBRARY__ or __GLIBC__?  Because many compiler toolchains (intel icc,
// clang, etc) will mimic gnuc and gnu toolchains by setting those defines, even though they
// are using different header files that don't actually need macro fixups.  Therefore we assume
// that linux will be using the headers.  Any other non-Linux target using GNU headers should
// manually specify USE_GLIBC_MACRO_FIXUP=1 in the makefile.
//
#if !defined(USE_GLIBC_MACRO_FIXUP)
#	if TARGET_LINUX
#		define USE_GLIBC_MACRO_FIXUP		1
#	else
#		define USE_GLIBC_MACRO_FIXUP		0
#	endif
#endif
// --------------------------------------------------------------------------------------

// --------------------------------------------------------------------------------------
// USING_CLANG_LTO
//
// This should be defined by the project file so that it matches other linker settings regarding
// debug symbols and such.  LTO is not always debug-information friendly and so the project file
// will disable those features when enabling LTO.  Forcing LTO to enabled here may yield an
// executable without debug information -- and forcing it to 1 may cause very long build times or
// crashes.
//
#if !defined(USING_CLANG_LTO)
#	define USING_CLANG_LTO		0
#endif
// --------------------------------------------------------------------------------------
