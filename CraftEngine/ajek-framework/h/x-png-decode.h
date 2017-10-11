
#pragma once

#include "x-types.h"
#include "x-BitmapData.h"

extern void png_LoadFromFile(xBitmapData& dest, const xString& filename);
extern void png_LoadFromFile(xBitmapData& dest, FILE* fp);

