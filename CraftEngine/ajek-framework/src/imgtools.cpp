
#include "PCH-framework.h"
#include "imgtools.h"

using namespace imgtool;

static __ai void _imt_copyscan(const u32* srcptr, u32* destptr, int copylen)
{
	for (int x=0; x<copylen; ++x, ++destptr, ++srcptr) {
		destptr[0] = srcptr[0];
	}
}

static __ai void _imt_copyscan_atst(const u32* srcptr, u32* destptr, int copylen, u32 color_cvt_to_alpha)
{
	for (int x=0; x<copylen; ++x, ++destptr, ++srcptr) {
		rgba32 src = (rgba32&)srcptr[0];
		if (color_cvt_to_alpha == (src.w32() & mask_rgb)) {
			src.a = 0x00;
		}
		destptr[0] = src.w32();
	}
}

void imgtool::ConvertOpaqueColorToAlpha(xBitmapData& image, const rgba32& color)
{
	todo("perform in-place conversion");
}

void imgtool::CutTex(xBitmapData& dest, const xBitmapData& src, int2 xy1, int2 xy2)
{
	bug_on(xy1.cmp_ge_any(src.size) || xy2.cmp_ge_any(src.size));
	bug_on(xy1.cmp_lt_any(0)		|| xy2.cmp_lt_any(0));

	int2 srcsize = xy2 - xy1;
	dest.size  = srcsize;
	dest.buffer.Resize(dest.size.x * dest.size.y * 4);

	// copy from src to dest
	// One pixel to the top/left/bottom/right has duplicated RGB info and alpha=0
	// This allows for artifact-free GPU bilinear filtering.


	int src_stride_px = src .size.x;
	int dst_stride_px = dest.size.x;

	u32* destptr = ((u32*)dest.buffer.GetPtr());
	u32* srcptr  = ((u32*)src .buffer.GetPtr()) + (xy1.y * src_stride_px) + xy1.x;

	int y;
	int syoffs = 0;
	int dyoffs = 0;

	for (y=0; y<dest.size.y; ++y, syoffs += src_stride_px, dyoffs += dst_stride_px) {
		_imt_copyscan(srcptr + syoffs, destptr + dyoffs, dest.size.x);
	}
}

void imgtool::CutTex_and_ConvertOpaqueColorToAlpha(xBitmapData& dest, const xBitmapData& src, int2 xy1, int2 xy2, const rgba32& color)
{
	bug_on(xy1.cmp_ge_any(src.size) || xy2.cmp_ge_any(src.size));
	bug_on(xy1.cmp_lt_any(0)		|| xy2.cmp_lt_any(0));

	int2 srcsize = xy2 - xy1;
	dest.size  = srcsize;
	dest.buffer.Resize(dest.size.x * dest.size.y * 4);

	// copy from src to dest
	// One pixel to the top/left/bottom/right has duplicated RGB info and alpha=0
	// This allows for artifact-free GPU bilinear filtering.

	auto color_native = color.w32();

	int src_stride_px = src .size.x;
	int dst_stride_px = dest.size.x;

	u32* destptr = ((u32*)dest.buffer.GetPtr());
	u32* srcptr  = ((u32*)src .buffer.GetPtr()) + (xy1.y * src_stride_px) + xy1.x;

	int y;
	int syoffs = 0;
	int dyoffs = 0;

	for (y=0; y<dest.size.y; ++y, syoffs += src_stride_px, dyoffs += dst_stride_px) {
		_imt_copyscan_atst(srcptr + syoffs, destptr + dyoffs, dest.size.x, color_native);
	}
}

int imgtool::AddTileToAtlas(TextureAtlas& dest, xBitmapData& src, int gpu_border)
{
	return 0;
}

void TextureAtlas::Init(const int2& tileSizePix, int texWidthHint) {
	m_tileSizePix	= tileSizePix;
	m_borderSizePix = 2;
}

void TextureAtlas::AddRows(int numrows) {
	if (numrows <= 0) return;

	m_bufferSizeInTiles.y += numrows;

	auto tileSizeFull	= m_tileSizePix + (m_borderSizePix * 2);
	auto sizeInPix		= (m_bufferSizeInTiles * tileSizeFull);
	auto newAllocSize	= sizeInPix.x * sizeInPix.y * 4;		// 32bpp

	m_buffer.Resize(newAllocSize);
}

int TextureAtlas::AllocTile()
{
	bug_on(m_tileSizePix.isEmpty());
	auto tileSizeFull = m_tileSizePix + (m_borderSizePix * 2);
	if (!m_bufferSizeInTiles.x) {

		// Init and allocate the texture atlas according to parameters.
		// Shader-capable GPUs generally don't have texture size restrictions.
		//  * some of them are more optimal dealing with texture sizes that are multiples of 8 or 16,
		//    but that can be handled when the atlas is uploaded to the GPU

		// Just some sanity limits, no specific hardware rationale for them  (yet). --jstine
		static const int MinTexWidth = 512;
		static const int MaxTexWidth = 4096;

		if (m_allocWidthHint) {
			m_bufferSizeInTiles.x = m_allocWidthHint;
		}
		else {
			int unhintedWidth = std::min((tileSizeFull.x * 16), MinTexWidth);
			if (unhintedWidth > 768 && unhintedWidth < 1024) {
				unhintedWidth = 1024;
			}
			if (unhintedWidth > 2048) {
				unhintedWidth = std::max((tileSizeFull.x * 12), MaxTexWidth);
			}
			m_bufferSizeInTiles.x = unhintedWidth / tileSizeFull.x;
		}
		m_bufferSizeInTiles.y = 0;
	}

	int hasRoomFor = m_bufferSizeInTiles.x * m_bufferSizeInTiles.y;
	if (m_numPasted >= hasRoomFor) {
		AddRows(1);
	}

	return m_numPasted++;
}
