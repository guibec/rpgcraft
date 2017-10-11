
#include "common-standard.h"
#include "x-png-decode.h"
#include "png.h"

void png_LoadFromFile(xBitmapData& dest, const xString& filename)
{
	bug_on (filename.IsEmpty());

	png_image image;
	png_color background = {0, 0xff, 0}; /* fully saturated green */

	image.version	= PNG_IMAGE_VERSION;
	image.opaque	= NULL;

	int beginResult = png_image_begin_read_from_file(&image, filename);
	log_and_abort_on(!beginResult, "libpng file error: %s", image.message);

	image.format	= PNG_FORMAT_RGBA;

	dest.width		= image.width;
	dest.height		= image.height;
	dest.buffer.Resize(PNG_IMAGE_SIZE(image));

	int finishResult = png_image_finish_read(&image, &background, dest.buffer.GetPtr(),
		0,			/*row_stride*/
		NULL		/*colormap for PNG_FORMAT_FLAG_COLORMAP */
	);

	log_and_abort_on(!finishResult, "libpng decode error: %s", image.message);
}


void png_LoadFromFile(xBitmapData& dest, FILE* fp)
{
	bug_on (!fp);

	png_image image;
	png_color background = {0, 0xff, 0}; /* fully saturated green */

	image.version	= PNG_IMAGE_VERSION;
	image.opaque	= NULL;

	int beginResult = png_image_begin_read_from_stdio(&image, fp);
	log_and_abort_on(!beginResult, "libpng file error: %s", image.message);

	image.format	= PNG_FORMAT_RGBA;

	dest.width		= image.width;
	dest.height		= image.height;
	dest.buffer.Resize(PNG_IMAGE_SIZE(image));

	int finishResult = png_image_finish_read(&image, &background, dest.buffer.GetPtr(),
		0,			/*row_stride*/
		NULL		/*colormap for PNG_FORMAT_FLAG_COLORMAP */
	);

	log_and_abort_on(!finishResult, "libpng decode error: %s", image.message);
}

