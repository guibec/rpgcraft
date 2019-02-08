
#include "PCH-framework.h"
#include "x-assertion.h"
#include "x-stdfile.h"
#include "x-png-encode.h"
#include "png.h"

void x_png_enc::Cleanup() {
    xFree(bmp);   bmp   = nullptr;
    xFree(lines); lines = nullptr;
}

x_png_enc& x_png_enc::WriteImage(const xBitmapDataRO& bitmap, bool reverseRGB_)
{
    return WriteImage(bitmap.buffer, bitmap.size.x, bitmap.size.y, 32);
}

x_png_enc& x_png_enc::WriteImage(const void* data, s32 width_, s32 height_, u32 bpp_, bool reverseRGB_)
{
    if (bpp_ != 24 && bpp_ != 32 && bpp_ != 48 && bpp_ != 64) {
        bug("Unsupported bpp=%d, valid formats are 24, 32, 48, and 64 bits-per-pixel.", bpp_);
        return *this;
    }

    Cleanup();
    width      = width_;
    height     = height_;
    bpp        = bpp_;
    reverseRGB = reverseRGB_;

    if (width <= 0 || height <= 0) {
        bug("Invalid width or height specified: %dx%d", width, height);
        return *this;
    }

    auto bytesPerPixel = bpp / 8;

    bmp        = (u8*) xRealloc(bmp,   height * width * bytesPerPixel);
    lines      = (u8**)xRealloc(lines, height * sizeof(u8*));

    for(int i = 0; i < height; i++) {
        lines[i] = &bmp[width * bytesPerPixel * i];
    }
    xMemCopy(bmp, data, width * height * bytesPerPixel);
    return *this;
}

x_png_enc& x_png_enc::Cvt64to32()
{
    if (bpp == 32) return *this;

    for(int y = 0; y < height; y++) {
        const auto* bmp_src = (u16*)(lines[y]);
              auto* bmp_dst = (u8*) (lines[y]);
        for(int x = 0; x < width*4;  x++) {
            bmp_dst[x] = bmp_src[x] >> 8;
        }
    }
    bpp = 32;
    return *this;
}

x_png_enc& x_png_enc::Cvt64to48()
{
    if (bpp == 48) return *this;

    for(int y = 0; y < height; y++) {
        const auto* bmp_src = (u16*)(lines[y]);
              auto* bmp_dst = (u16*)(lines[y]);
        for(int x = 0; x < width;  x++) {
            bmp_dst[(x*3)+0] = bmp_src[(x*4)+0];
            bmp_dst[(x*3)+1] = bmp_src[(x*4)+1];
            bmp_dst[(x*3)+2] = bmp_src[(x*4)+2];
        }
    }
    bpp = 48;
    return *this;
}

x_png_enc& x_png_enc::Cvt64to24()
{
    if (bpp == 24) return *this;

    for(int y = 0; y < height; y++) {
        const auto* bmp_src = (u16*)(lines[y]);
              auto* bmp_dst = (u8*) (lines[y]);
        for(int x = 0; x < width;  x++) {
            bmp_dst[(x*3)+0] = bmp_src[(x*4)+0] >> 8;
            bmp_dst[(x*3)+1] = bmp_src[(x*4)+1] >> 8;
            bmp_dst[(x*3)+2] = bmp_src[(x*4)+2] >> 8;
        }
    }
    bpp = 24;
    return *this;
}

x_png_enc& x_png_enc::Cvt32to24()
{
    if (bpp == 24) return *this;

    for(int y = 0; y < height; y++) {
        const auto* bmp_src = (u8*)(lines[y]);
              auto* bmp_dst = (u8*)(lines[y]);
        for(int x = 0; x < width;  x++) {
            bmp_dst[(x*3)+0] = bmp_src[(x*4)+0];
            bmp_dst[(x*3)+1] = bmp_src[(x*4)+1];
            bmp_dst[(x*3)+2] = bmp_src[(x*4)+2];
        }
    }
    bpp = 24;
    return *this;
}


x_png_enc& x_png_enc::ClearAlphaChannel(u8 alpha)
{
    if (bpp == 24 || bpp == 48) {
        // nothing to do - no alpha channel exists.
        return *this;
    }

    if (bpp != 32) {
        bug("Don't know how to clear alpha channel of non-32bpp image data.");
        return *this;
    }

    for(int y = 0; y < height; y++) {
    auto* bmp_dst = (u8*) (lines[y]);
    for(int x = 0; x < width;  x++) {
        bmp_dst[x*4 + 3] = alpha;
    }}
    return *this;
}

int x_png_enc::SaveImage(const char* filename, int compression_level)
{
    if (width <= 0 || height <= 0) return false;

    //if (bpp != 32) {
    //    bug("Currently only 32bit pixels supported!");
    //    return false;
    //}

    FILE* fp = xFopen(filename, "wb");

    if(!fp || !bmp || !lines) {
        bug("ERROR: Could not open '%s'", filename);
        return 1;
    }

    png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

    if (!png_ptr) {
        bug("ERROR: Could not create png_structp");
        fclose(fp);
        return 2;
    }

    png_infop info_ptr = png_create_info_struct(png_ptr);

    if(!info_ptr) {
        bug("ERROR: Could not create png_infop");
        png_destroy_write_struct(&png_ptr, (png_infopp)NULL);
        fclose(fp);
        return 3;
    }

    if (setjmp(png_jmpbuf(png_ptr))) {
        bug("ERROR: Internal error writing png");
        png_destroy_write_struct(&png_ptr, &info_ptr);
        fclose(fp);
        return 4;
    }

    int bitsPerColor;
    int colorType;

    switch(bpp) {
        case 24: bitsPerColor = 8;  colorType = PNG_COLOR_TYPE_RGB;  break;
        case 32: bitsPerColor = 8;  colorType = PNG_COLOR_TYPE_RGBA; break;
        case 48: bitsPerColor = 16; colorType = PNG_COLOR_TYPE_RGB;  break;
        case 64: bitsPerColor = 16; colorType = PNG_COLOR_TYPE_RGBA; break;
    }

    png_set_compression_level(png_ptr, compression_level);
    png_set_IHDR(png_ptr, info_ptr, width, height, bitsPerColor, colorType, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

    u32 transform_flags = PNG_TRANSFORM_IDENTITY | (reverseRGB ? PNG_TRANSFORM_BGR : 0);
    png_set_rows (png_ptr, info_ptr, lines);
    png_init_io  (png_ptr, fp);
    png_write_png(png_ptr, info_ptr, transform_flags, 0);

    png_destroy_write_struct(&png_ptr, &info_ptr);
    fclose(fp);
    return 0;
}
