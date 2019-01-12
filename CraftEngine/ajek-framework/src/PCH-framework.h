
#if !defined(USE_PRECOMPILED_HEADER)
#   error USE_PRECOMPILED_HEADER has not been defined.
#endif

#if USE_PRECOMPILED_HEADER
#include <cstdio>
#include <cstdlib>
#include <cstdarg>

#if defined(__GLIBC__)
// GLIBC has abusive use of double-underscore variable names.
#   include <memory>
#endif

#include "x-types.h"
#include "x-simd.h"

#include "x-stl.h"

#include "x-assertion.h"
#include "x-string.h"
#include "x-stdlib.h"
#endif
