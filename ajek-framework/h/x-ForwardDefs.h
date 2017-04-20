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

union	u128;

// --------------------------------------------------------
//  x-string.h
// --------------------------------------------------------

class	xString;
class	xStringTokenizer;
class	toUTF16;
class	toUTF8;

// --------------------------------------------------------
//  x-thread.h
// --------------------------------------------------------

class	xSemaphore;
class	xMutex;
class	xCondVar;
class	xScopedMutex;


class	BaseTextFileWriter;
