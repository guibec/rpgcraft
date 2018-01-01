
#pragma once

#include "x-TargetConfig.h"
#include "x-ForwardDefs.h"

//
// GCC_CHECK_VER: Useful macro for decoding GCC's obtuse version system.
//
#define GCC_CHECK_VER(major, minor)     ( defined(__GNUC__) && \
    ( (__GNUC__ > major) || ((__GNUC__ == major) && (__GNUC_MINOR__ >= minor)) ) )

#if defined(_MSC_VER)
    // Disable a whole lot of utterly useless warnings.  Most of these are only actually enabled
    // when the compiler is set to Level W4, which is pretty solid evidence that using W4 isn't very
    // helpful in the first place.  Easy fix is to just avoid using W4 (ever), but no harm in explicit
    // disablings in case there are a couple useful warnings in the w4 zone.  --jstine

#   pragma warning(disable: 4100)       // unreferenced formal parameter  (irrelevant to optimizing compiler performance)
#   pragma warning(disable: 4065)       // switch statement has 'default' but no case labels  (who cares?)
#   pragma warning(disable: 4060)       // switch statement contains no 'case' or 'default' labels  (who cares?)
#   pragma warning(disable: 4127)       // conditional expression is a constant  (extremely common and valid coding technique)
#   pragma warning(disable: 4200)       // nonstandard extension used: zero-sized array (supported by all modern compilers)
#   pragma warning(disable: 4201)       // nonstandard extension used: nameless struct/union (supported by all modern compilers)
#   pragma warning(disable: 4702)       // unreachable code (extremely common and valid coding technique)
#   pragma warning(disable: 4324)       // structure padding due to __declspec (that's the whole idea of using __declspec(align))
#   pragma warning(disable: 4800)       // 'u32' : forcing value to bool 'true' or 'false' (perf problem?  really?  compiler generates a test/setx pair for this in the WORST case)
#   pragma warning(disable: 4141)       // 'inline': used more than once  (which happens when using __forceinline -- indicating that microsoft still doesn't quite understand that `inline` in C++ is TU directive and not an optimization directive)
    // conversion from int to (smaller_type), possible loss of data
    // (happens all the time in x64, and explicit typecasts to avoid these warnings are redundant and annoying)
#   pragma warning(disable: 4244)
#   pragma warning(disable: 4267)

// Microsoft loves their 32/64 bit conversion warnings.  One can only imagine how many bad coding practices have been littered through
// out Microsoft/Windows codebase to merit 1. roughly 6 warnings dedicated to trying to catch such things, and 2. having the warnings
// put in at Warning Level #1 (always enabled).
// Worst part: fixing these warning with macros actually does a disservice to other superior code analysis tools, namely Valgrind and
// MSVC's own "Smaller Type Check" option.  Other great example of how this warning can lead to bad code design is Microsoft's own
// HIWORD()/LOWORD() macros, which mute this warning but also kill the sign bit.  Oops.  Conclusion: disable these, forever.

#   pragma warning(disable: 4311)       // pointer truncation from 'void *' to 'u32'
#   pragma warning(disable: 4302)       // truncation from 'void *' to 'u32'

#   pragma warning(disable: 4250)       // 'class' inherits 'base::interface' via dominance
#endif

// --------------------------------------------------------------------------------------
//                                Common Type Includes
// --------------------------------------------------------------------------------------

#ifndef __STDC_LIMIT_MACROS
#   define __STDC_LIMIT_MACROS      // ... for those who prefer C99 limits opposed to C++ std::numeric_limits mess.
#endif

#if defined(_MSC_VER) && (_MSC_VER < 1600)
#   include "msw/pstdint.h"
#else
#   include <stdint.h>
#endif

// --------------------------------------------------------------------------------------
//                               Core/Atomic Type Defines
// --------------------------------------------------------------------------------------

typedef uint8_t         u8;
typedef uint16_t        u16;
typedef uint32_t        u32;
typedef uint64_t        u64;

typedef int8_t          s8;
typedef int16_t         s16;
typedef int32_t         s32;
typedef int64_t         s64;

#if UNICODE
    typedef wchar_t     wchar;
#else
    typedef char        wchar;
#endif


#if TARGET_x64 && USE_64BIT_UINT
    typedef uint64_t    uint;
    typedef int64_t     sint;
#else
    typedef uint32_t    uint;
    typedef int32_t     sint;
#endif

#if TARGET_x64
    typedef int64_t     IntPtr;
    typedef uint64_t    UIntPtr;
#else
    typedef int32_t     IntPtr;
    typedef uint32_t    UIntPtr;
#endif

typedef IntPtr          sptr;
typedef UIntPtr         uptr;
typedef s64             x_off_t;        // file offsets are always 64 bits.


typedef void VoidFunc();

// -------------------------------------------------------------
//                Common Intrinsic Includes
// -------------------------------------------------------------
// Notes:
//   intrin_x86 is a GCC-specific compatibility implementation of microsoft/intel-specific
//   intrinsics, such as _InterlockedExchange and __debugbreak. (normally provided by <intrin.h>
//

#if TARGET_ORBIS
#   include <scebase.h>
#   include <kernel.h>
#   include "../../thirdparty/gcc/intrin_x86.h"
#elif defined(__GNUC__)
// this includes all XMMs, which is really not what I want, but GCC isn't keen on letting us jsut include ia32intrin.h.  :(
#   include <x86intrin.h>
#   include "intrin_x86.h"      // thirdparty provision, mimics microsoft/intel <intrin.h>
#else
#   include <intrin.h>
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

#   define sealed                   final
#   define __alwaysinline           __attribute__((always_inline))
#   define __forceinline            __attribute__((used,always_inline))
#   define __classinline            __attribute__((used,always_inline))

#   define __unreachable()          __builtin_unreachable()
#   define __RESTRICT__             __restrict
#   define __threadlocal            __thread
#   define EXPECT(a, b)             __builtin_expect((a), (b))

#   define __aligned_x              __aligned(16)

#   define __noreturn               __attribute__((noreturn))
#   define __optimize(n)
#   define _msc_pragma(str)
#   if (SCE_ORBIS_SDK_VERSION >> 16) < 0x0200
#       define  __is_trivially_copyable(x)  (std::is_trivially_copyable<T>::value)
#       define  __is_standard_layout(x)     (std::is_standard_layout<T>::value)
#   endif
#   define __verify_fmt(fmtpos, vapos)

#   define foreach( typeVar, srclist )  for( typeVar : srclist)

#   define PRAGMA_OPTIMIZE_BEGIN()
#   define PRAGMA_OPTIMIZE_END()

// These are defined in sys/param.h, included by kernel.h.  For shame.  --jstine

#   undef  MIN
#   undef  MAX

// ================================================================================================
#elif defined(__GNUC__)

#if !GCC_CHECK_VER( 4, 5 )
#   define __builtin_unreachable()  ((void)0)
#endif

#if !GCC_CHECK_VER( 4, 7 )
#   define override
#   define sealed
#else
#   define sealed                   final
#endif

    // GNU C++ Compliant macro defines

#   define __alwaysinline           __attribute__((__always_inline__))
#   define __forceinline            __attribute__((used,always_inline))
#   define __classinline            __attribute__((used,always_inline))
#   define __noinline               __attribute__((noinline))
#   define __used

#   define __unreachable()          __builtin_unreachable()
#   define __RESTRICT__             __restrict
#   define __threadlocal            __thread
#   define EXPECT(a, b)             __builtin_expect((a), (b))

#   define __aligned(x)             __attribute__((aligned(x)))
#   define __aligned_x              __attribute__((aligned(16)))
#   define __packed                 __attribute__((packed))

#   define __noreturn               __attribute__((noreturn))
#   define __optimize(n)            __attribute__((optimize(n)))
#   define _msc_pragma(str)
#   define __unused                 __attribute__((unused))

// GCC 4.7 still has no support for the std:: version of this.  The builtin will probably
// be around indefinitely so let's just use it unconditionally for now.
#   define  __is_trivially_copyable(x)      __has_trivial_copy(x)

// A lot of GLIBC headers use __xi and __unused as variable names (sigh), so we have to
// undef and redef them accordingly, as needed.  Therefore create both __UNUSED and __unused,
// so that __unused can re re-defined after including problematic headers.

#if USE_GLIBC_MACRO_FIXUP
#   define __UNUSED                 __attribute__((unused))
#endif

#   define __verify_fmt(fmtpos, vapos)  __attribute__ ((format (printf, fmtpos, vapos)))

#   define foreach( typeVar, srclist )  for( typeVar : srclist)

#   define PRAGMA_OPTIMIZE_BEGIN()
#   define PRAGMA_OPTIMIZE_END()

// ================================================================================================
#elif defined(_MSC_VER)

    // Microsoft Visual Studio compliant macro defines

#   define __classinline
#   define __alwaysinline           __forceinline
#   define __noinline               __declspec(noinline)
#   define __used

#   define __unreachable()          __assume(false)
#   define __RESTRICT__             __restrict
#   define __threadlocal            __declspec(thread)
#   define EXPECT(a,b)              (a)

#   define __aligned(x)             __declspec(align(x))
#   define __aligned_x              __declspec(align(16))
#   define __packed

#   define __noreturn               __declspec(noreturn)
#   define __optimize(n)
#   define __UNUSED
#   define __unused
#   define _msc_pragma(str)         __pragma( str )

// VS 2012 supports is_trivially_copyable!
// VS 2010... not so much.
#if _MSC_VER >= 0x1700
#   define  __is_trivially_copyable(x)      (std::is_trivially_copyable<T>::value)
#else
#   define  __is_trivially_copyable(x)      (true)
#endif

// MSVC has no printf format verification fanciness :(
#   define __verify_fmt(fmtpos, vapos)

#   define foreach( typeVar, srclist )  for each( typeVar in srclist )

#   define PRAGMA_OPTIMIZE_BEGIN()  __pragma( optimize( "gt", on ) )
#   define PRAGMA_OPTIMIZE_END()    __pragma( optimize( "", on ) )

#if TARGET_x64
    typedef s64         ssize_t;
#else
    typedef s32         ssize_t;
#endif

#endif

#define EXPECT_FALSE(a)             EXPECT((a), (false))
#define EXPECT_TRUE(a)              EXPECT((a), (true))

#define BEGIN_GPU_DATA_STRUCTS      __pragma(pack(push,1))
#define END_GPU_DATA_STRUCTS        __pragma(pack(pop))

// --------------------------------------------------------------------------------------
//  pragma_todo    (macro)
// --------------------------------------------------------------------------------------
// Used to produce handy-dandy messages like:
//    1> C:\Source\Project\main.cpp(47): Reminder: Fix this problem!

#define macro_Stringize( L )            #L
#define macro_MakeString( M, L )        M(L)
#define _pragmahelper_Line              macro_MakeString( macro_Stringize, __LINE__ )
#define _pragmahelper_location          __FILE__ "(" _pragmahelper_Line "): "

#if !defined(SHOW_PRAGMA_TODO)
#   define SHOW_PRAGMA_TODO         1
#endif

#if SHOW_PRAGMA_TODO
#   if defined(_MSC_VER)
#       define pragma_todo(...)     __pragma    (message        (_pragmahelper_location "-TODO- " __VA_ARGS__))
#   else
#       define pragma_todo(...)     _Pragma     (STR(GCC warning(_pragmahelper_location "-TODO- " __VA_ARGS__)))
#   endif
#else
#   define pragma_todo(...)
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
#   define FMT_SIZET    "%Iu"
#else
#   define FMT_SIZET    "%zu"
#endif

#if TARGET_x64 && defined(__GNUC__)
#   define FMT_U64      "%lu"
#   define FMT_S64      "%ld"
#else
#   define FMT_U64      "%llu"
#   define FMT_S64      "%lld"
#endif

// --------------------------------------------------------------------------------------
//                               Endian / Byteswap Usefulness
// --------------------------------------------------------------------------------------

#ifdef _MSC_VER
#   define xSWAP16(in)  _byteswap_ushort(in)
#   define xSWAP32(in)  _byteswap_ulong(in)
#   define xSWAP64(in)  _byteswap_uint64(in)
#elif defined(__clang__)
#   include "machine/endian.h"
#   define xSWAP16(in)  __bswap16(in)
#   define xSWAP32(in)  __bswap32(in)
#   define xSWAP64(in)  __bswap64(in)
#else
#   include "byteswap.h"
#   define xSWAP16(in)  bswap_16(in)
#   define xSWAP32(in)  bswap_32(in)
#   define xSWAP64(in)  bswap_64(in)
#endif



// --------------------------------------------------------------------------------------
//                                Static Constant Helpers
// --------------------------------------------------------------------------------------
//
// Notes:
//  * use of s64 helps avoid unwanted promotion to u64, which can happen during mult/div operations
//    when u64 constants are combined with other s32 or s64 operands.
//
static const s64 _1kb   = 0x400;
static const s64 _2kb   = 0x800;
static const s64 _4kb   = 0x1000;
static const s64 _6kb   = 0x1800;
static const s64 _8kb   = 0x2000;
static const s64 _16kb  = 0x4000;
static const s64 _32kb  = 0x8000;
static const s64 _64kb  = _16kb * 4;
static const s64 _96kb  = _1kb  * 96;
static const s64 _128kb = _64kb * 2;
static const s64 _256kb = _64kb * 4;
static const s64 _512kb = _1kb * 512;

static const s64 _1mb   = _256kb * 4;
static const s64 _2mb   = _1mb * 2;
static const s64 _4mb   = _1mb * 4;
static const s64 _6mb   = _1mb * 6;
static const s64 _8mb   = _1mb * 8;
static const s64 _16mb  = _1mb * 16;
static const s64 _20mb  = _1mb * 20;
static const s64 _24mb  = _1mb * 24;
static const s64 _32mb  = _1mb * 32;
static const s64 _64mb  = _1mb * 64;
static const s64 _128mb = _1mb * 128;
static const s64 _192mb = _1mb * 192;
static const s64 _256mb = _1mb * 256;
static const s64 _512mb = _1mb * 512;
static const s64 _1gb   = _512mb * 2;
static const s64 _2gb   = _1gb * 2;
static const s64 _4gb   = _1gb * 4;



enum BinaryBits8 : u8 {
    B00000000= 0,
    B00000001= 1,
    B00000010= 2,
    B00000011= 3,
    B00000100= 4,
    B00000101= 5,
    B00000110= 6,
    B00000111= 7,
    B00001000= 8,
    B00001001= 9,
    B00001010= 10,
    B00001011= 11,
    B00001100= 12,
    B00001101= 13,
    B00001110= 14,
    B00001111= 15,
    B00010000= 16,
    B00010001= 17,
    B00010010= 18,
    B00010011= 19,
    B00010100= 20,
    B00010101= 21,
    B00010110= 22,
    B00010111= 23,
    B00011000= 24,
    B00011001= 25,
    B00011010= 26,
    B00011011= 27,
    B00011100= 28,
    B00011101= 29,
    B00011110= 30,
    B00011111= 31,
    B00100000= 32,
    B00100001= 33,
    B00100010= 34,
    B00100011= 35,
    B00100100= 36,
    B00100101= 37,
    B00100110= 38,
    B00100111= 39,
    B00101000= 40,
    B00101001= 41,
    B00101010= 42,
    B00101011= 43,
    B00101100= 44,
    B00101101= 45,
    B00101110= 46,
    B00101111= 47,
    B00110000= 48,
    B00110001= 49,
    B00110010= 50,
    B00110011= 51,
    B00110100= 52,
    B00110101= 53,
    B00110110= 54,
    B00110111= 55,
    B00111000= 56,
    B00111001= 57,
    B00111010= 58,
    B00111011= 59,
    B00111100= 60,
    B00111101= 61,
    B00111110= 62,
    B00111111= 63,
    B01000000= 64,
    B01000001= 65,
    B01000010= 66,
    B01000011= 67,
    B01000100= 68,
    B01000101= 69,
    B01000110= 70,
    B01000111= 71,
    B01001000= 72,
    B01001001= 73,
    B01001010= 74,
    B01001011= 75,
    B01001100= 76,
    B01001101= 77,
    B01001110= 78,
    B01001111= 79,
    B01010000= 80,
    B01010001= 81,
    B01010010= 82,
    B01010011= 83,
    B01010100= 84,
    B01010101= 85,
    B01010110= 86,
    B01010111= 87,
    B01011000= 88,
    B01011001= 89,
    B01011010= 90,
    B01011011= 91,
    B01011100= 92,
    B01011101= 93,
    B01011110= 94,
    B01011111= 95,
    B01100000= 96,
    B01100001= 97,
    B01100010= 98,
    B01100011= 99,
    B01100100= 100,
    B01100101= 101,
    B01100110= 102,
    B01100111= 103,
    B01101000= 104,
    B01101001= 105,
    B01101010= 106,
    B01101011= 107,
    B01101100= 108,
    B01101101= 109,
    B01101110= 110,
    B01101111= 111,
    B01110000= 112,
    B01110001= 113,
    B01110010= 114,
    B01110011= 115,
    B01110100= 116,
    B01110101= 117,
    B01110110= 118,
    B01110111= 119,
    B01111000= 120,
    B01111001= 121,
    B01111010= 122,
    B01111011= 123,
    B01111100= 124,
    B01111101= 125,
    B01111110= 126,
    B01111111= 127,
    B10000000= 128,
    B10000001= 129,
    B10000010= 130,
    B10000011= 131,
    B10000100= 132,
    B10000101= 133,
    B10000110= 134,
    B10000111= 135,
    B10001000= 136,
    B10001001= 137,
    B10001010= 138,
    B10001011= 139,
    B10001100= 140,
    B10001101= 141,
    B10001110= 142,
    B10001111= 143,
    B10010000= 144,
    B10010001= 145,
    B10010010= 146,
    B10010011= 147,
    B10010100= 148,
    B10010101= 149,
    B10010110= 150,
    B10010111= 151,
    B10011000= 152,
    B10011001= 153,
    B10011010= 154,
    B10011011= 155,
    B10011100= 156,
    B10011101= 157,
    B10011110= 158,
    B10011111= 159,
    B10100000= 160,
    B10100001= 161,
    B10100010= 162,
    B10100011= 163,
    B10100100= 164,
    B10100101= 165,
    B10100110= 166,
    B10100111= 167,
    B10101000= 168,
    B10101001= 169,
    B10101010= 170,
    B10101011= 171,
    B10101100= 172,
    B10101101= 173,
    B10101110= 174,
    B10101111= 175,
    B10110000= 176,
    B10110001= 177,
    B10110010= 178,
    B10110011= 179,
    B10110100= 180,
    B10110101= 181,
    B10110110= 182,
    B10110111= 183,
    B10111000= 184,
    B10111001= 185,
    B10111010= 186,
    B10111011= 187,
    B10111100= 188,
    B10111101= 189,
    B10111110= 190,
    B10111111= 191,
    B11000000= 192,
    B11000001= 193,
    B11000010= 194,
    B11000011= 195,
    B11000100= 196,
    B11000101= 197,
    B11000110= 198,
    B11000111= 199,
    B11001000= 200,
    B11001001= 201,
    B11001010= 202,
    B11001011= 203,
    B11001100= 204,
    B11001101= 205,
    B11001110= 206,
    B11001111= 207,
    B11010000= 208,
    B11010001= 209,
    B11010010= 210,
    B11010011= 211,
    B11010100= 212,
    B11010101= 213,
    B11010110= 214,
    B11010111= 215,
    B11011000= 216,
    B11011001= 217,
    B11011010= 218,
    B11011011= 219,
    B11011100= 220,
    B11011101= 221,
    B11011110= 222,
    B11011111= 223,
    B11100000= 224,
    B11100001= 225,
    B11100010= 226,
    B11100011= 227,
    B11100100= 228,
    B11100101= 229,
    B11100110= 230,
    B11100111= 231,
    B11101000= 232,
    B11101001= 233,
    B11101010= 234,
    B11101011= 235,
    B11101100= 236,
    B11101101= 237,
    B11101110= 238,
    B11101111= 239,
    B11110000= 240,
    B11110001= 241,
    B11110010= 242,
    B11110011= 243,
    B11110100= 244,
    B11110101= 245,
    B11110110= 246,
    B11110111= 247,
    B11111000= 248,
    B11111001= 249,
    B11111010= 250,
    B11111011= 251,
    B11111100= 252,
    B11111101= 253,
    B11111110= 254,
    B11111111= 255
};

#define elif(a)                 else if(a)

// Allows nice formatting of "large" hex numerical values, namely 64 bit addresses.
// Ex: AddrConstr_0x(0x20,1000,4000)
#define AddrConstr_0x(page, upper32, lower32)   ((s64(0x##page) << 32) | (s64(0x##upper32) << 16) | (s64(0x##lower32)))

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
#   define INLINE_ACCESSORS         1
#endif

#if !defined(INLINE_FUNCTIONS)
#   if TARGET_DEBUG
#       define INLINE_FUNCTIONS     0
#   else
#       define INLINE_FUNCTIONS     1
#   endif
#endif

#if !defined(INLINE_TEMPLATES)
#   if TARGET_FINAL
#       define INLINE_TEMPLATES     1
#   else
#       define INLINE_TEMPLATES     0
#   endif
#endif

#define __ni                    __noinline

#if INLINE_ACCESSORS
#   define __ai                 __alwaysinline
#   define __eai                __forceinline
#else
#   define __ai
#   define __eai
#endif

#if INLINE_FUNCTIONS
#   define __xi                 __alwaysinline
#   define __exi                __forceinline
#   define __cxi                __classinline
#else
#   define __xi                 __noinline
#   define __exi                __noinline
#   define __cxi                __noinline
#endif

#if INLINE_TEMPLATES
#   define __ti                 __alwaysinline
#   define __cti                __classinline
#   define __eti                __forceinline
#else
#   define __ti                 __noinline
#   define __cti                __noinline
#   define __eti                __noinline
#endif

#if TARGET_FINAL
#   define __fi                 __alwaysinline
#else
#   define __fi                 __noinline
#endif

#if USE_GLIBC_MACRO_FIXUP
    // See x-TargetConfig.h for details...
#   if INLINE_FUNCTIONS
#       define __ALWAYS_INLINE_FUNCTION     __attribute__((__always_inline__))
#   else
#       define __ALWAYS_INLINE_FUNCTION     __noinline
#   endif
#endif


#define TOUCH(a)            ((void)(a))
#define bulkof(a)           (sizeof(a) / sizeof((a)[0]))
#define _XSTRING(num)       #num
#define STRING(num)         _XSTRING(num)
#define IsWithin(p, a, l)   (((u64(p)) - (u64(a))) < (u64(l)))

// --------------------------------------------------------------------------------------
// Support for MSVC's At-Runtime "Smaller Type Check" feature.
// The feature is only useful/effective if intentional value truncations are macro-
// encapsulated to mask off the truncated bits first.
//
#if !MASK_SMALLER_TYPE_CASTS

#   define _NCS8(a)         ((s8)(a))
#   define _NCS16(a)        ((s16)(a))
#   define _NCS32(a)        ((s32)(a))
#   define _NCU8(a)         ((u8)(a))
#   define _NCU16(a)        ((u16)(a))
#   define _NCU32(a)        ((u32)(a))

    template< typename T >
    __ai T _NC( u64 src ) { return src; }

#else

#   define _NCS8(a)         ((s8)((a) & 0xff))
#   define _NCS16(a)        ((s16)((a) & 0xffff))
#   define _NCS32(a)        ((s32)((a) & 0xffffffff))
#   define _NCU8(a)         ((u8)((a) & 0xffU))
#   define _NCU16(a)        ((u16)((a) & 0xffffU))
#   define _NCU32(a)        ((u32)((a) & 0xffffffffU))

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
        u16 h0, h1;
    };

    struct
    {
        u8  b0,   b1,   b2,   b3;
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
    u64     _u64;
    s64     _s64;

    struct
    {
        u32 lo;
        u32 hi;
    };

    struct
    {
        u32 w0, w1;
    };

    struct
    {
        u16 h0, h1, h2, h3;
    };

    struct
    {
        u8  b0,   b1,   b2,   b3;
        u8  b4,   b5,   b6,   b7;
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

inline __ai bool IsInDisp8(u32 x)   { return 0xFFFFFF80 <= x || x <= 0x7F; }
inline __ai bool IsInInt32(u64 x)   { return s64(s32(x)) == s64(x); }
inline __ai bool IsInInt32u(u64 x)  { return (x <= 0xFFFFFFFFUL); }

// -------------------------------------------------------------
//                 Diagnostic Tools
// -------------------------------------------------------------

// %s(%d) format is Visual Studio friendly -- allows double-clicking of the output
// message to go to source code instance of assertion.  Linux builds targeted for
// eclipse may benefit from a slightly different format.
#define __FILEPOS__         __FILE__ "(" STRING(__LINE__) "): "

#if defined(__GNUG__)
#   define __FUNCTION_NAME__        __PRETTY_FUNCTION__
#else
    // MSVC still defines __FUNCTION__ as a string literal as opposed to a const char*
    // (as prescribed by the C++ standard).  Therefore we explicitly typecast it so that
    // msvc errors will mimic those seen on gcc/clang.
#   define __FUNCTION_NAME__        ((const char*)__FUNCTION__)
#endif

#define DECLARE_MODULE_NAME_CLASS( name )   static      __unused const char* const s_ModuleName;
#define DECLARE_MODULE_NAME_LOCAL( name )   static      __unused const char* const s_ModuleName = name;
#define DECLARE_MODULE_NAME( name )         namespace { __unused const char* const s_ModuleName = name; }

// -------------------------------------------------------------
//                     C++ Class Macros
// -------------------------------------------------------------

#define CALL_MEMBER_FN(object,ptrToMember)  ((object).*(ptrToMember))

#define NONCOPYABLE_OBJECT(classname)                   \
    explicit classname(const classname&) = delete;      \
    classname& operator=(const classname&) = delete


// -------------------------------------------------------------
// CaseReturnString( caseName )
// -------------------------------------------------------------
// Neat!  Returns the case option as a string matching precisely the case label.
// Useful for logging hardware registers and for converting sparse enumerations
// into strings (enums where simple char* arrays fail).
//
#define CaseReturnString( caseName )        case caseName: return # caseName
#define CaseReturnString2( ofs, caseName )  case caseName: return # caseName + (ofs)
