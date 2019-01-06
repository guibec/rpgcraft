
#pragma once

#include "x-types.h"
#include "x-string.h"
#include "x-BitmapData.h"

extern void png_LoadFromFile(xBitmapData& dest, const xString& filename);
extern void png_LoadFromFile(xBitmapData& dest, FILE* fp, const xString& filename_for_error_msg_only=xString());
