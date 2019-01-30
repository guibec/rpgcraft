
#include "PCH-framework.h"
#include "x-png-decode.h"
#include "x-stdfile.h"

#include "png.h"

void png_LoadFromFile(xBitmapData& dest, const xString& filename)
{
    bug_on (filename.IsEmpty());
    bug_on (!xPathIsUniversal(filename));       // paths should be converted at

    auto* fp = xFopen(filename, "rb");
    x_abort_on (!fp, "png_LoadFromFile('%s') error: %s", filename.c_str(), strerror(errno));
    png_LoadFromFile(dest, fp, filename);
    fclose(fp);
}

static xString png_errmsg_helper(const xString& filename, const FILE* fp)
{
    if (filename.IsEmpty()) {
        return xFmtStr("fp=%s", cPtrStr(fp));
    }
    else {
        return xFmtStr("'%s'", filename.c_str());
    }
}

// filename_for_error_msg_only - optional filename parameter for use in error messages only.
void png_LoadFromFile(xBitmapData& dest, FILE* fp, const xString& filename_for_error_msg_only)
{
    bug_on (!fp);
    const xString& filename = filename_for_error_msg_only;

    png_image image;
    png_color background = {0, 0xff, 0}; /* fully saturated green */

    image.version   = PNG_IMAGE_VERSION;
    image.opaque    = NULL;

    int beginResult = png_image_begin_read_from_stdio(&image, fp);
    x_abort_on(!beginResult, "png_LoadFromFile(%s) read error: %s", png_errmsg_helper(filename, fp), image.message);

    // assume any negative size indicates some invalid object state.
    dest.size = uint2 { image.width, image.height };
    x_abort_on(dest.size.cmp_any() < 0, "png_LoadFromFile(%s) error: invalid object state dest.size={ %s }", png_errmsg_helper(filename, fp), cHexStr(dest.size));

    image.format    = PNG_FORMAT_RGBA;
    dest.buffer.Resize(PNG_IMAGE_SIZE(image));

    int finishResult = png_image_finish_read(&image, &background, dest.buffer.GetPtr(),
        0,          /*row_stride*/
        NULL        /*colormap for PNG_FORMAT_FLAG_COLORMAP */
    );

    x_abort_on(!finishResult, "png_LoadFromFile(%s) decode error: %s", png_errmsg_helper(filename, fp), image.message);
}

