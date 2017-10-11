
#pragma once

#include "png.h"
#include "zlib.h"

class x_png {

private:
	u8*  bmp;
	u8** lines;
	s32  width;
	s32  height;
	u32  bbp;
	bool reverseRGB;

public:
	x_png()  { xMemZero(*this); }
	~x_png() { Cleanup(); }

	void Cleanup() {
		if (bmp)   { delete[] bmp;   bmp   = nullptr; }
		if (lines) { delete[] lines; lines = nullptr; }
	}

	void WriteImage(const void* data, s32 width_, s32 height_, u32 bbp_ = 32, bool reverseRGB_ = false) {
		if (bbp_ != 32) {
			bug("Currently only 32bit pixels supported!");
			return;
		}

		Cleanup();
		width      = width_;
		height     = height_;
		bbp        = bbp_;
		reverseRGB = reverseRGB_;

		if (width <= 0 || height <= 0) {
			bug("Invalid width or height specified: %dx%d", width, height);
			return;
		}

		bmp        = new u8 [height * width * 4];
		lines      = new u8*[height];
		for(int i = 0; i < height; i++) {
			lines[i] = &bmp[width * 4 * i];
		}
		xMemCopy(bmp, data, width * height * 4);

		u8* bmp_y = bmp;
		for(int y = 0; y < height; y++, bmp_y += (width*4)) {
		for(int x = 0; x < width;  x++) {
			bmp_y[x*4 + 3] = 255; // opaque
		}}
	}

	// Compression level is from 0~9 (0=none, 9=max)
	int SaveImage(const char* filename, int compression_level = 9) {
		if (width <= 0 || height <= 0) return false;

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

		png_set_compression_level(png_ptr, compression_level);
		png_set_IHDR(png_ptr, info_ptr, width, height, 8, PNG_COLOR_TYPE_RGBA, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

		u32 transform_flags = PNG_TRANSFORM_IDENTITY | (reverseRGB ? PNG_TRANSFORM_BGR : 0);
		png_set_rows (png_ptr, info_ptr, lines);
		png_init_io  (png_ptr, fp);
		png_write_png(png_ptr, info_ptr, transform_flags, 0);

		png_destroy_write_struct(&png_ptr, &info_ptr);
		fclose(fp);
		return 0;
	}
};
