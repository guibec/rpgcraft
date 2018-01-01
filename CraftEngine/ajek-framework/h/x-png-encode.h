
#pragma once

#include "x-types.h"
#include "x-BitmapData.h"
#include "imgtools.h"

class x_png_enc {

private:
    u8*  bmp;
    u8** lines;
    s32  width;
    s32  height;
    u32  bbp;
    bool reverseRGB;

public:
     x_png_enc()    { xMemZero(*this); }
    ~x_png_enc()    { Cleanup(); }

    void            Cleanup             ();
    x_png_enc&      WriteImage          (const xBitmapDataRO& bitmap, bool reverseRGB_ = false);
    x_png_enc&      WriteImage          (const void* data, s32 width_, s32 height_, u32 bbp_ = 32, bool reverseRGB_ = false);
    int             SaveImage           (const char* filename, int compression_level = 9);      // Compression level is from 0~9 (0=none, 9=max)
    x_png_enc&      ClearAlphaChannel   (u8 alpha = 255);

};

