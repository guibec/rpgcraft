
#pragma once

#include "x-types.h"

//
// Provides <string> and <list>.  Given its own header because MSVC 2010 has an annoying bug:
//
//    When using the MSVCRT DLLs (_DLL==1) with CPP exception support disabled (_HAS_EXCEPTIONS==0), the
//    headers generate superfluous warnings.  This is amusing since the specific code generating the
//    warnings are std::exception classes, which aren't especially useful when exceptions are disabled.
//

// Oh look!  GLIBC (gnu/linux) ALSO has an annoying problem:
//
//    GCC uses __xi as a variable name internally (random.tcc), so we must include <algorithm>
//    before including xTypes.
//
// rant: Why is GCC mangling names for code within the STL namespace anyway?  The whole point and
//       purpose of namespaces is to allow for the use of unmangled names, thus freeing up the land
//       of mangled names for use by MACROS.  Using mangled names for local names within a namespace
//       just results in:
//         1. ugly code (see random.tcc, it's horrible)
//         2. a bunch of otherwise nice macro names that aren't easily usable now since ugly
//            mangled-name headers depend on them.
//       --jstine

#if USE_GLIBC_MACRO_FIXUP
#	undef __unused
#	undef __xi
#	undef __ti
#endif

#if defined(_MSC_VER) && (_DLL == 1) && (_HAS_EXCEPTIONS == 0)
#	pragma warning(disable: 4275)
#endif

#include <string>
#include <list>
#include <algorithm>
#include <memory>

template< typename T > 
inline __ai T xBoundsCheck(const T& src, const T& lower, const T& upper) {
	return std::min( std::max(src, lower), upper);
}

#ifdef _MSC_VER
#	pragma warning(default: 4275)
#endif

#if USE_GLIBC_MACRO_FIXUP
#	define __unused		__UNUSED
#	define __xi			__ALWAYS_INLINE_FUNCTION
#	define __ti			__ALWAYS_INLINE_FUNCTION
#endif
