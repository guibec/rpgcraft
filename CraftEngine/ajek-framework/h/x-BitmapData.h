
#pragma once

#include "x-ScopedMalloc.h"
#include "x-simd.h"

// Simple is good: Bitmap data is always R8G8B8A8 format
struct xBitmapData
{
	int2				size;
	ScopedMalloc<u8>	buffer;
};
