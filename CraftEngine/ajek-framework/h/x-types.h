
#pragma once

#include "x-TargetConfig.h"
#include "x-ForwardDefs.h"

//
// GCC_CHECK_VER: Useful macro for decoding GCC's obtuse version system.
//
#define GCC_CHECK_VER(major, minor)		( defined(__GNUC__) && \
	( (__GNUC__ > major) || ((__GNUC__ == major) && (__GNUC_MINOR__ >= minor)) ) )

#if defined(_MSC_VER)
	// Disable a whole lot of utterly useless warnings.  Most of these are only actually enabled
	// when the compiler is set to Level W4, which is pretty solid evidence that using W4 isn't very
	// helpful in the first place.  Easy fix is to just avoid using W4 (ever), but no harm in explicit
	// disablings in case there are a couple useful warnings in the w4 zone.  --jstine

#	pragma warning(disable: 4100)		// unreferenced formal parameter  (irrelevant to optimizing compiler performance)
#	pragma warning(disable: 4065)		// switch statement has 'default' but no case labels  (who cares?)
#	pragma warning(disable: 4060)		// switch statement contains no 'case' or 'default' labels  (who cares?)
#	pragma warning(disable: 4127)		// conditional expression is a constant  (extremely common and valid coding technique)
#	pragma warning(disable: 4200)		// nonstandard extension used: zero-sized array (supported by all modern compilers)
#	pragma warning(disable: 4201)		// nonstandard extension used: nameless struct/union (supported by all modern compilers)
#	pragma warning(disable: 4702)		// unreachable code (extremely common and valid coding technique)
#	pragma warning(disable: 4324)		// structure padding due to __declspec (that's the whole idea of using __declspec(align))
#	pragma warning(disable: 4800)		// 'u32' : forcing value to bool 'true' or 'false' (perf problem?  really?  compiler generates a test/setx pair for this in the WORST case)
#	pragma warning(disable: 4141)		// 'inline': used more than once  (which happens when using __forceinline -- indicating that microsoft still doesn't quite understand that `inline` in C++ is TU directive and not an optimization directive)
	// conversion from int to (smaller_type), possible loss of data
	// (happens all the time in x64, and explicit typecasts to avoid these warnings are redundant and annoying)
#	pragma warning(disable: 4244)
#	pragma warning(disable: 4267)

// Microsoft loves their 32/64 bit conversion warnings.  One can only imagine how many bad coding practices have been littered through
// out Microsoft/Windows codebase to merit 1. roughly 6 warnings dedicated to trying to catch such things, and 2. having the warnings
// put in at Warning Level #1 (always enabled).
// Worst part: fixing these warning with macros actually does a disservice to other superior code analysis tools, namely Valgrind and
// MSVC's own "Smaller Type Check" option.  Other great example of how this warning can lead to bad code design is Microsoft's own
// HIWORD()/LOWORD() macros, which mute this warning but also kill the sign bit.  Oops.  Conclusion: disable these, forever.

#	pragma warning(disable: 4311)		// pointer truncation from 'void *' to 'u32'
#	pragma warning(disable: 4302)		// truncation from 'void *' to 'u32'

# 	pragma warning(disable: 4250)		// 'class' inherits 'base::interface' via dominance
#endif

// --------------------------------------------------------------------------------------
//                                Common Type Includes
// --------------------------------------------------------------------------------------

#ifndef __STDC_LIMIT_MACROS
#	define __STDC_LIMIT_MACROS		// ... for those who prefer C99 limits opposed to C++ std::numeric_limits mess.
#endif

#if defined(_MSC_VER) && (_MSC_VER < 1600)
#	include "msw/pstdint.h"
#else
#	include <stdint.h>
#endif

// --------------------------------------------------------------------------------------
//                               Core/Atomic Type Defines
// --------------------------------------------------------------------------------------

typedef uint8_t			u8;
typedef uint16_t		u16;
typedef uint32_t		u32;
typedef uint64_t		u64;

typedef int8_t			s8;
typedef int16_t			s16;
typedef int32_t			s32;
typedef int64_t			s64;

#if UNICODE
	typedef wchar_t		wchar;
#else
	typedef char		wchar;
#endif


#if TARGET_x64 && USE_64BIT_UINT
	typedef uint64_t	uint;
	typedef int64_t		sint;
#else
	typedef uint32_t	uint;
	typedef int32_t		sint;
#endif

#if TARGET_x64
	typedef int64_t		IntPtr;
	typedef uint64_t	UIntPtr;
#else
	typedef int32_t		IntPtr;
	typedef uint32_t	UIntPtr;
#endif

typedef IntPtr			sptr;
typedef UIntPtr			uptr;
typedef s64				x_off_t;		// file offsets are always 64 bits.


typedef void VoidFunc();

// -------------------------------------------------------------
//                Common Intrinsic Includes
// -------------------------------------------------------------
// Notes:
//   intrin_x86 is a GCC-specific compatibility implementation of microsoft/intel-specific
//   intrinsics, such as _InterlockedExchange and __debugbreak. (normally provided by <intrin.h>
//

#if TARGET_ORBIS
#	include <scebase.h>
#	include <kernel.h>
#	include "../../thirdparty/gcc/intrin_x86.h"
#elif defined(__GNUC__)
// this includes all XMMs, which is really not what I want, but GCC isn't keen on letting us jsut include ia32intrin.h.  :(
#	include <x86intrin.h>
#	include "intrin_x86.h"		// thirdparty provision, mimics microsoft/intel <intrin.h>
#else
#	include <intrin.h>
#endif

// -------------------------------------------------------------
//                Compiler-specific Defines
// -------------------------------------------------------------
//
// # always_inline is considered gcc territory, which in this case means "must
//   be paired with the 'inline' keyword."  Since it must be paired with 'inline',
//   and since C++ has no comprehension of "external inlines" (such a thing only
//   exists in C99), its usefulness is limited to source files in headers/inl, or
//   static functions not referenced outside the current translation unit.
//
// # __forceinline is considered MSVC territory, which in this case means "can be
//   paired with "extern" for inter-procedural forced inlines."  Very handy in this
//   modern day, where compiling and linking an entire source tree all-at-once is
//   hardly a costly affair.
//
// # __aligned_x should be set to whatever provides the best cache performance for the
//   target architecture.  Most Intel chipsets require 16.  Future architectures may
//   prefer 32, or may be able to use 8.  Ex:
//     * AVX allows for unaligned 128 bit loads/stores, and thus 8-based alignment may use
//       cachelines more efficiently).

#if defined(__clang__)

//
// Note: ORBIS SDK already defines several things that we've also be defining here (yay!)
//       Such as __unused, __aligned(), __noinline, etc.
//

#	define sealed					final
#	define __alwaysinline			__attribute__((always_inline))
#	define __forceinline			__attribute__((used,always_inline))
#	define __classinline			__attribute__((used,always_inline))

#	define __unreachable()			__builtin_unreachable()
#	define __RESTRICT__				__restrict
#	define __threadlocal			__thread
#	define EXPECT(a, b)				__builtin_expect((a), (b))

#	define __aligned_x				__aligned(16)

#	define __noreturn				__attribute__((noreturn))
#	define __optimize(n)
#	define _msc_pragma(str)
#	if (SCE_ORBIS_SDK_VERSION >> 16) < 0x0200
#		define	__is_trivially_copyable(x)	(std::is_trivially_copyable<T>::value)
#		define	__is_standard_layout(x)		(std::is_standard_layout<T>::value)
#	endif
#	define __verify_fmt(fmtpos, vapos)

#	define foreach( typeVar, srclist ) 	for( typeVar : srclist)

#	define PRAGMA_OPTIMIZE_BEGIN()
#	define PRAGMA_OPTIMIZE_END()

// These are defined in sys/param.h, included by kernel.h.  For shame.  --jstine

#	undef  MIN
#	undef  MAX

// ================================================================================================
#elif defined(__GNUC__)

#if !GCC_CHECK_VER( 4, 5 )
#	define __builtin_unreachable()	((void)0)
#endif

#if !GCC_CHECK_VER( 4, 7 )
#	define override
#	define sealed
#else
#	define sealed					final
#endif

	// GNU C++ Compliant macro defines

#	define __alwaysinline			__attribute__((__always_inline__))
#	define __forceinline			__attribute__((used,always_inline))
#	define __classinline			__attribute__((used,always_inline))
#	define __noinline				__attribute__((noinline))
#	define __used

#	define __unreachable()			__builtin_unreachable()
#	define __RESTRICT__				__restrict
#	define __threadlocal			__thread
#	define EXPECT(a, b)				__builtin_expect((a), (b))

#	define __aligned(x)				__attribute__((aligned(x)))
#	define __aligned_x				__attribute__((aligned(16)))
#	define __packed					__attribute__((packed))

#	define __noreturn				__attribute__((noreturn))
#	define __optimize(n)			__attribute__((optimize(n)))
#	define _msc_pragma(str)
#	define __unused					__attribute__((unused))

// GCC 4.7 still has no support for the std:: version of this.  The builtin will probably
// be around indefinitely so let's just use it unconditionally for now.
#	define	__is_trivially_copyable(x)		__has_trivial_copy(x)

// A lot of GLIBC headers use __xi and __unused as variable names (sigh), so we have to
// undef and redef them accordingly, as needed.  Therefore create both __UNUSED and __unused,
// so that __unused can re re-defined after including problematic headers.

#if USE_GLIBC_MACRO_FIXUP
#	define __UNUSED					__attribute__((unused))
#endif

#	define __verify_fmt(fmtpos, vapos)  __attribute__ ((format (printf, fmtpos, vapos)))

#	define foreach( typeVar, srclist ) 	for( typeVar : srclist)

#	define PRAGMA_OPTIMIZE_BEGIN()
#	define PRAGMA_OPTIMIZE_END()

// ================================================================================================
#elif defined(_MSC_VER)

	// Microsoft Visual Studio compliant macro defines

#	define __classinline
#	define __alwaysinline			__forceinline
#	define __noinline				__declspec(noinline)
#	define __used

#	define __unreachable()			__assume(false)
#	define __RESTRICT__				__restrict
#	define __threadlocal			__declspec(thread)
#	define EXPECT(a,b)				(a)

#	define __aligned(x)				__declspec(align(x))
#	define __aligned_x				__declspec(align(16))
#	define __packed

#	define __noreturn				__declspec(noreturn)
#	define __optimize(n)
#	define __UNUSED
#	define __unused
#	define _msc_pragma(str)			__pragma( str )

// VS 2012 supports is_trivially_copyable!
// VS 2010... not so much.
#if _MSC_VER >= 0x1700
#	define	__is_trivially_copyable(x)		(std::is_trivially_copyable<T>::value)
#else
#	define	__is_trivially_copyable(x)		(true)
#endif

// MSVC has no printf format verification fanciness :(
#	define __verify_fmt(fmtpos, vapos)

#	define foreach( typeVar, srclist ) 	for each( typeVar in srclist )

#	define PRAGMA_OPTIMIZE_BEGIN()	__pragma( optimize( "gt", on ) )
#	define PRAGMA_OPTIMIZE_END()	__pragma( optimize( "", on ) )

#if TARGET_x64
	typedef s64			ssize_t;
#else
	typedef s32			ssize_t;
#endif

#endif

#define EXPECT_FALSE(a)				EXPECT((a), (false))
#define EXPECT_TRUE(a)				EXPECT((a), (true))

#define BEGIN_GPU_DATA_STRUCTS		__pragma(pack(push,1))
#define END_GPU_DATA_STRUCTS		__pragma(pack(pop))

// --------------------------------------------------------------------------------------
//  pragma_todo    (macro)
// --------------------------------------------------------------------------------------
// Used to produce handy-dandy messages like:
//    1> C:\Source\Project\main.cpp(47): Reminder: Fix this problem!

#define macro_Stringize( L )			#L
#define macro_MakeString( M, L )		M(L)
#define _pragmahelper_Line				macro_MakeString( macro_Stringize, __LINE__ )
#define _pragmahelper_location			__FILE__ "(" _pragmahelper_Line "): "

#if !defined(SHOW_PRAGMA_TODO)
#	define SHOW_PRAGMA_TODO			1
#endif

#if SHOW_PRAGMA_TODO
#	if defined(_MSC_VER)
#		define pragma_todo(...)		__pragma	(message        (_pragmahelper_location "-TODO- " __VA_ARGS__))
#	else
#		define pragma_todo(...)		_Pragma		(STR(GCC warning(_pragmahelper_location "-TODO- " __VA_ARGS__)))
#	endif
#else
#	define pragma_todo(...)
#endif


// --------------------------------------------------------------------------------------
// FMT_SIZET / FMT_U64 / FMT_S64
// --------------------------------------------------------------------------------------
//  # Recommended engineers use cHexStr()/cDecStr() instead (see x-string.h).  It Just Works, in
//    a cleaner and better way, especially if the fundamental types being displayed are promoted
//    or demoted.
//  # MSVC does not yet support 'z' modifier on printf, so this macro is needed until its C99
//    compliance develops into a less ugly flower.
//  # MSVC and GCC finally agree on %lld in 32 bit land, which is not much of a consolation since
//    they now do NOT agree in x64 land.
//
#ifdef _MSC_VER
#	define FMT_SIZET	"%Iu"
#else
#	define FMT_SIZET	"%zu"
#endif

#if TARGET_x64 && defined(__GNUC__)
#	define FMT_U64		"%lu"
#	define FMT_S64		"%ld"
#else
#	define FMT_U64		"%llu"
#	define FMT_S64		"%lld"
#endif

// --------------------------------------------------------------------------------------
//                               Endian / Byteswap Usefulness
// --------------------------------------------------------------------------------------

#ifdef _MSC_VER
#	define xSWAP16(in)	_byteswap_ushort(in)
#	define xSWAP32(in)	_byteswap_ulong(in)
#	define xSWAP64(in)	_byteswap_uint64(in)
#elif defined(__clang__)
#	include "machine/endian.h"
#	define xSWAP16(in)	__bswap16(in)
#	define xSWAP32(in)	__bswap32(in)
#	define xSWAP64(in)	__bswap64(in)
#else
#	include "byteswap.h"
#	define xSWAP16(in)	bswap_16(in)
#	define xSWAP32(in)	bswap_32(in)
#	define xSWAP64(in)	bswap_64(in)
#endif



// --------------------------------------------------------------------------------------
//                                Static Constant Helpers
// --------------------------------------------------------------------------------------
//
// Notes:
//  * use of s64 helps avoid unwanted promotion to u64, which can happen during mult/div operations
//    when u64 constants are combined with other s32 or s64 operands.
//
static const s64 _1kb	= 0x400;
static const s64 _2kb	= 0x800;
static const s64 _4kb	= 0x1000;
static const s64 _6kb	= 0x1800;
static const s64 _8kb	= 0x2000;
static const s64 _16kb	= 0x4000;
static const s64 _32kb	= 0x8000;
static const s64 _64kb	= _16kb * 4;
static const s64 _96kb	= _1kb  * 96;
static const s64 _128kb	= _64kb * 2;
static const s64 _256kb	= _64kb * 4;
static const s64 _512kb	= _1kb * 512;

static const s64 _1mb	= _256kb * 4;
static const s64 _2mb	= _1mb * 2;
static const s64 _4mb	= _1mb * 4;
static const s64 _6mb	= _1mb * 6;
static const s64 _8mb	= _1mb * 8;
static const s64 _16mb	= _1mb * 16;
static const s64 _20mb	= _1mb * 20;
static const s64 _24mb	= _1mb * 24;
static const s64 _32mb	= _1mb * 32;
static const s64 _64mb	= _1mb * 64;
static const s64 _128mb	= _1mb * 128;
static const s64 _192mb	= _1mb * 192;
static const s64 _256mb	= _1mb * 256;
static const s64 _512mb	= _1mb * 512;
static const s64 _1gb	= _512mb * 2;
static const s64 _2gb	= _1gb * 2;
static const s64 _4gb	= _1gb * 4;

#define elif(a)					else if(a)

// Allows nice formatting of "large" hex numerical values, namely 64 bit addresses.
// Ex: AddrConstr_0x(0x20,1000,4000)
#define AddrConstr_0x(page, upper32, lower32)	((s64(0x##page) << 32) | (s64(0x##upper32) << 16) | (s64(0x##lower32)))

// --------------------------------------------------------------------------------------
//  __ai / __xi / __ti (macros)
// --------------------------------------------------------------------------------------
// See "docs/inlining directives.txt" for full details.
// Provided shorthands for __forceinline which are conditional on the build target:
//
//  __ai : shorthand for 'always_inline', which can be enabled even in debug targets by telling
//         the compiler to honor inline directives.  It is intended for use on class member
//         accessors, where 'skip over' behavior during code traces is desirable in debug builds.
//         This can also improve debug-built execution speeds considerably.
//
//  __xi : inline for TARGET_QA or TARGET_FINAL targets, regarded as __noinline for all debug
//         targets.  Intended for use on most non-accessor class members implemented in header
//         files, as otherwise most of these will behave as though they are __ai (see below).
//
//  __ti : inline enabled for TARGET_FINAL only, intended for use only with nested template class
//         and function inlines, which create serious problems for the MSVC linker and can make
//         compiling builds for debug/qa use a slow marathon.
//

#if !defined(INLINE_ACCESSORS)
#	define INLINE_ACCESSORS			1
#endif

#if !defined(INLINE_FUNCTIONS)
#	if TARGET_DEBUG
#		define INLINE_FUNCTIONS		0
#	else
#		define INLINE_FUNCTIONS		1
#	endif
#endif

#if !defined(INLINE_TEMPLATES)
#	if TARGET_FINAL
#		define INLINE_TEMPLATES		1
#	else
#		define INLINE_TEMPLATES		0
#	endif
#endif

#define	__ni					__noinline

#if INLINE_ACCESSORS
#	define __ai					__alwaysinline
#	define __eai				__forceinline
#else
#	define __ai
#	define __eai
#endif

#if INLINE_FUNCTIONS
#	define __xi					__alwaysinline
#	define __exi				__forceinline
#	define __cxi				__classinline
#else
#	define __xi					__noinline
#	define __exi				__noinline
#	define __cxi				__noinline
#endif

#if INLINE_TEMPLATES
#	define __ti					__alwaysinline
#	define __cti				__classinline
#	define __eti				__forceinline
#else
#	define __ti					__noinline
#	define __cti				__noinline
#	define __eti				__noinline
#endif

#if TARGET_FINAL
#	define __fi					__alwaysinline
#else
#	define __fi					__noinline
#endif

#if USE_GLIBC_MACRO_FIXUP
	// See x-TargetConfig.h for details...
#	if INLINE_FUNCTIONS
#		define __ALWAYS_INLINE_FUNCTION		__attribute__((__always_inline__))
#	else
#		define __ALWAYS_INLINE_FUNCTION		__noinline
#	endif
#endif


#define TOUCH(a)			((void)(a))
#define bulkof(a)			(sizeof(a) / sizeof((a)[0]))
#define _XSTRING(num)		#num
#define STRING(num)			_XSTRING(num)
#define IsWithin(p, a, l)	(((u64(p)) - (u64(a))) < (u64(l)))

// --------------------------------------------------------------------------------------
// Support for MSVC's At-Runtime "Smaller Type Check" feature.
// The feature is only useful/effective if intentional value truncations are macro-
// encapsulated to mask off the truncated bits first.
//
#if !MASK_SMALLER_TYPE_CASTS

#	define _NCS8(a)			((s8)(a))
#	define _NCS16(a)		((s16)(a))
#	define _NCS32(a)		((s32)(a))
#	define _NCU8(a)			((u8)(a))
#	define _NCU16(a)		((u16)(a))
#	define _NCU32(a)		((u32)(a))

	template< typename T >
	__ai T _NC( u64 src ) { return src; }

#else

#	define _NCS8(a)			((s8)((a) & 0xff))
#	define _NCS16(a)		((s16)((a) & 0xffff))
#	define _NCS32(a)		((s32)((a) & 0xffffffff))
#	define _NCU8(a)			((u8)((a) & 0xffU))
#	define _NCU16(a)		((u16)((a) & 0xffffU))
#	define _NCU32(a)		((u32)((a) & 0xffffffffU))

	template< typename T >
	__ai T _NC( u64 src )
	{
		static const u64 mask = (u32)((1ULL << (8*sizeof(T))) - 1);
		return (T)(src & mask);
	}
#endif

// ----------------------------------------------------------------------------
//   Helpful Packed Type Unions
// ----------------------------------------------------------------------------
union IntPack32
{
	u32 _u32;
	s32 _s32;

	struct
	{
		u16	h0, h1;
	};

	struct
	{
		u8	b0,   b1,   b2,   b3;
	};

	operator u32() const
	{
		return _u32;
	}

	IntPack32& operator=( const u32& src )
	{
		_u32 = src;
		return *this;
	}
};

union IntPack64
{
	u64		_u64;
	s64		_s64;

	struct
	{
		u32 lo;
		u32 hi;
	};

	struct
	{
		u32	w0, w1;
	};

	struct
	{
		u16	h0, h1, h2, h3;
	};

	struct
	{
		u8	b0,   b1,   b2,   b3;
		u8	b4,   b5,   b6,   b7;
	};

	static IntPack64 fromPtr( const void* src )
	{
		IntPack64 retval;
		retval._s64 = (IntPtr) src;
		return retval;
	}

	operator u64() const
	{
		return _u64;
	}

	IntPack64& operator=( const u64& src )
	{
		_u64 = src;
		return *this;
	}
};

inline __ai bool IsInDisp8(u32 x)	{ return 0xFFFFFF80 <= x || x <= 0x7F; }
inline __ai bool IsInInt32(u64 x)	{ return s64(s32(x)) == s64(x); }
inline __ai bool IsInInt32u(u64 x)	{ return (x <= 0xFFFFFFFFUL); }

// -------------------------------------------------------------
//                 Diagnostic Tools
// -------------------------------------------------------------

// %s(%d) format is Visual Studio friendly -- allows double-clicking of the output
// message to go to source code instance of assertion.  Linux builds targeted for
// eclipse may benefit from a slightly different format.
#define __FILEPOS__			__FILE__ "(" STRING(__LINE__) "): "

#if defined(__GNUG__)
#	define __FUNCTION_NAME__		__PRETTY_FUNCTION__
#else
	// MSVC still defines __FUNCTION__ as a string literal as opposed to a const char*
	// (as prescribed by the C++ standard).  Therefore we explicitly typecast it so that
	// msvc errors will mimic those seen on gcc/clang.
#	define __FUNCTION_NAME__		((const char*)__FUNCTION__)
#endif

#define DECLARE_MODULE_NAME_CLASS( name )	static		__unused const char* const s_ModuleName;
#define DECLARE_MODULE_NAME_LOCAL( name )	static		__unused const char* const s_ModuleName = name;
#define DECLARE_MODULE_NAME( name )			namespace { __unused const char* const s_ModuleName = name; }

// -------------------------------------------------------------
//                     C++ Class Macros
// -------------------------------------------------------------

#define CALL_MEMBER_FN(object,ptrToMember)  ((object).*(ptrToMember))

#define NONCOPYABLE_OBJECT(classname)					\
	explicit classname(const classname&) = delete;		\
	classname& operator=(const classname&) = delete


// -------------------------------------------------------------
// CaseReturnString( caseName )
// -------------------------------------------------------------
// Neat!  Returns the case option as a string matching precisely the case label.
// Useful for logging hardware registers and for converting sparse enumerations
// into strings (enums where simple char* arrays fail).
//
#define CaseReturnString( caseName )		case caseName: return # caseName
#define CaseReturnString2( ofs, caseName )	case caseName: return # caseName + (ofs)
