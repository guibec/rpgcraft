
#pragma once

#include "x-types.h"
#include "x-BitmapData.h"
#include "imgtools.h"

class x_png_enc {

private:
    u8*  bmp        = nullptr;
    u8** lines      = nullptr;
    s32  width      = 0;
    s32  height     = 0;
    u32  bpp        = 0;
    bool reverseRGB = 0;

public:
    ~x_png_enc()    { Cleanup(); }

    void            Cleanup             ();
    x_png_enc&      WriteImage          (const xBitmapDataRO& bitmap, bool reverseRGB_ = false);
    x_png_enc&      WriteImage          (const void* data, s32 width_, s32 height_, u32 bpp_ = 32, bool reverseRGB_ = false);
    int             SaveImage           (const char* filename, int compression_level = 9);      // Compression level is from 0~9 (0=none, 9=max)
    x_png_enc&      ClearAlphaChannel   (u8 alpha = 255);
    x_png_enc&      Cvt64to32           ();
    x_png_enc&      Cvt64to24           ();
    x_png_enc&      Cvt64to48           ();
    x_png_enc&      Cvt32to24           ();

};

