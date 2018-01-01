
//
// msw-redtape.h
//
// Including <windows.h> is not always a cut-and-dry procedure.  This header file helps out
// by ensuring that our types and windows types are included in the right order, and that lots
// of other unwanted crap can get undefined.
//

#pragma once

#include "x-types.h"
#include "x-simd.h"
#include "x-assertion.h"

#if TARGET_MSW

// Default to requiring Windows 7 or newer.  There's a good chance the emu will build/run with
// 0x600 or 0x500... but... that's only going to be considered on a need-to-support basis.
#   ifndef _WIN32_WINNT
#       define _WIN32_WINNT     0x601
#   endif

#   include <windows.h>
#   include "msw-error.h"

// Seriously, Microsoft and their complete disregard for code interoperability ...
//  (can't wait for them to switch from macros to inline functions for their Ascii/WideChar
//   patchthru functions)

#   undef GetCurrentTime
#   undef LoadImage

    extern void msw_AttachToParentConsole();
#endif


