#pragma once

// x-ForwardDefs.h
//
// This header is for common/shared utility class stuff only!
// If the class you're doing a forward def for is not part of this lib, then
// please forward define it elsewhere local to the project in which it is
// implemented.  Thanks!
//

// --------------------------------------------------------
//  x-simd.h
// --------------------------------------------------------

union   u128;
union   int2;
union   int4;
union   float2;
union   float4;

// --------------------------------------------------------
//  x-string.h
// --------------------------------------------------------

class   xString;
class   xStringTokenizer;
class   toUTF16;
class   toUTF8;

// --------------------------------------------------------
//  x-unixpath.h
// --------------------------------------------------------
struct xUnixPath;

// --------------------------------------------------------
//  x-thread.h
// --------------------------------------------------------

class   xSemaphore;
class   xMutex;
class   xCondVar;
class   xScopedMutex;

// --------------------------------------------------------
//  ajek-script.h
// --------------------------------------------------------

struct AjekScriptEnv;
struct lua_string;
