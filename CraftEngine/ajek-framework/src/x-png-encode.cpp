
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

// Writes image data into a heap-allocated buffer.  This function is meant to provide a means by which image
// data can be off-loaded onto a separate thread for processing and then encoding.
x_png_enc& x_png_enc::WriteImage(const void* data, s32 width_, s32 height_, s32 bpp_, bool reverseRGB_)
{
    bug_on(width_ <= 0 || height_ <= 0 || bpp_ <= 0, "WriteImage(width=%d, height=%d, bpp=%d): one or more invalid parameters.", width_, height_, bpp_);
    switch(bpp_) {
        case 64: break;
        case 48: break;
        case 32: break;
        case 24: break;

        // TODO: Add support for grayscale formats, which would be described as 8 and 16 bits per pixel.
        //       Grayscale formats would be ideal for dumping alpha channels and depth stencils.

        default:
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

// Converts 64 or 48 bit source data to 32 or 24 bit source data
// If the src format is know to be 64 bit and the alpha channel also needs to be removed then
// it's better to use CvtTo24(), which both downsamples and strips the alpha channel in
// a single operation.
x_png_enc& x_png_enc::DownSample()
{
    if (bpp == 32) return *this;
    if (bpp == 24) return *this;

    auto compsPerPixel = (bpp / 16);
    for(int y = 0; y < height; y++) {
        const auto* bmp_src = (u16*)(lines[y]);
              auto* bmp_dst = (u8*) (lines[y]);
        for(int x = 0; x < width*compsPerPixel;  x++) {
            bmp_dst[x] = bmp_src[x] >> 8;
        }
    }
    bpp /= 2;
    return *this;
}

x_png_enc& x_png_enc::Cvt64to48()
{
    if (bpp == 48) return *this;
    if (bpp != 64) {
        bug("Invalid x_png_enc object state, expected bpp=32 but bpp=%d", bpp);
        return *this;
    }

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

x_png_enc& x_png_enc::Cvt32to24()
{
    if (bpp == 24) return *this;
    if (bpp != 32) {
        bug("Invalid x_png_enc object state, expected bpp=32 but bpp=%d", bpp);
        return *this;
    }

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

x_png_enc& x_png_enc::Cvt64to24()
{
    if (bpp == 24) return *this;
    if (bpp != 64) {
        bug("Invalid x_png_enc object state, expected bpp=64 but bpp=%d", bpp);
        return *this;
    }

    auto compsPerPixel = bpp / 8;
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

// Minimizes the operational size of input data, by ensuring 8 bits per color and no alpha
// channel data.  This is typically the best option to use when saving non-grayscale texture
// data.
x_png_enc& x_png_enc::CvtTo24()
{
    if (bpp == 24) return *this;
    if (bpp == 32) return Cvt32to24();
    if (bpp == 48) return DownSample();
    if (bpp == 64) return Cvt64to24();

    bug("Unsupported image type bpp=%d", bpp);
    return *this;
}

x_png_enc& x_png_enc::StripAlphaChannel()
{
    if (bpp == 48 || bpp == 24) return *this;

    if (bpp == 64) return Cvt64to48();
    if (bpp == 32) return Cvt32to24();

    bug("Don't know how to strip alpha from image when bpp=%d", bpp);
    return *this;
}


// Clears alpha channel without deleting it outright.
// The function is grandfathered in for now, but might get removed in the future.
// This is of limited use, since it's almost always better to strip the alpha channel completely,
// since that results in faster encoding time and smaller png files, and it's trivial to re-insert
// the alpha channel later on load.
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
    if (width <= 0 || height <= 0)  return false;
    if (!bmp || !lines)             return false;

    FILE* fp = xFopen(filename, "wb");  Defer(fp && (fclose(fp), fp = nullptr));

    if(!fp) {
        bug("ERROR: Could not open '%s'", filename);
        return 1;
    }

    png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);

    if (!png_ptr) {
        bug("ERROR: Could not create png_structp");
        return 2;
    }

    png_infop info_ptr = png_create_info_struct(png_ptr);
    Defer(png_destroy_write_struct(&png_ptr, &info_ptr));

    if(!info_ptr) {
        bug("ERROR: Could not create png_infop");
        return 3;
    }

    if (setjmp(png_jmpbuf(png_ptr))) {
        bug("ERROR: Internal error writing png");
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

    return 0;
}
