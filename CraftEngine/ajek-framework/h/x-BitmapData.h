
#pragma once

#include "x-ScopedMalloc.h"

struct xBitmapData
{
	int					width;
	int					height;
	ScopedMalloc<u8>	buffer;
};
