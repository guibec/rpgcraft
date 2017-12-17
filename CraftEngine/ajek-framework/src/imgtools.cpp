
#include "PCH-framework.h"
#include "x-ThrowContext.h"
#include "imgtools.h"

using namespace imgtool;

DECLARE_MODULE_NAME("imgtool");
DECLARE_MODULE_THROW(xThrowModule_Bridge);

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

static void _cuttex_prepDest(xBitmapData& dest, const xBitmapData& src, int2 xy1, int2 xy2)
{
	bool paramInvalid =
			((xy1.cmp_any() >= xy2	) || (xy2.cmp_any() >  src.size	))
		||	((xy1.cmp_any() <  0	) || (xy2.cmp_any() <= 0		));

	throw_abort_on(paramInvalid, "X/Y parameters are outside the bounds of the source image (xy1=%s xy2=%s src=%s",
		cDecStr(xy1), cDecStr(xy2), cDecStr(src.size)
	);

	dest.size  = xy2 - xy1;
	dest.buffer.Resize(dest.size.x * dest.size.y * 4);
}

void imgtool::CutTex(xBitmapData& dest, const xBitmapData& src, int2 xy1, int2 xy2)
{
	// copy from src to dest
	// One pixel to the top/left/bottom/right has duplicated RGB info and alpha=0
	// This allows for artifact-free GPU bilinear filtering.

	_cuttex_prepDest(dest, src, xy1, xy2);

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
	// copy from src to dest
	// One pixel to the top/left/bottom/right has duplicated RGB info and alpha=0
	// This allows for artifact-free GPU bilinear filtering.

	_cuttex_prepDest(dest, src, xy1, xy2);

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

int imgtool::AddEmptyTileToAtlas(TextureAtlas& dest)
{
	int  tileId		= dest.AllocTile();
	auto tileSize	= dest.m_tileSizePix;
	auto tilePos	= dest.GetTilePosPix(tileId);
	auto tilePosEnd = tilePos + tileSize;
	auto destSize	= dest.GetSizePix();

	u32* dstptr		= (u32*)dest.GetRawPtr32()	+ (tilePos.y * destSize.x)	+ tilePos.x;

	for(int y=0; y<dest.m_tileSizePix.y; ++y, dstptr+=destSize.x)
	{
		auto* dptr	= dstptr;
		memset(dptr, 0, dest.m_tileSizePix.x * 4);
	}
	return 0;
}

int imgtool::AddTileToAtlas(TextureAtlas& dest, const xBitmapData& src, const int2& srcpos)
{
	int  tileId		= dest.AllocTile();
	auto tileSize	= dest.m_tileSizePix;
	auto tilePos	= dest.GetTilePosPix(tileId);
	auto tilePosEnd = tilePos + tileSize;
	auto destSize	= dest.GetSizePix();

	throw_abort_on((srcpos+tileSize).cmp_any() > src.size,
		"Requested tile cut is outside bounds of source image"
	);

	const	u32* srcptr		= (u32*)src.buffer.GetPtr()	+ (srcpos.y  * src.size.x)	+ srcpos.x;
			u32* dstptr		= (u32*)dest.GetRawPtr32()	+ (tilePos.y * destSize.x)	+ tilePos.x;

	for(int y=0; y<dest.m_tileSizePix.y; ++y, srcptr+=src.size.x, dstptr+=destSize.x)
	{
		auto* sptr	= srcptr;
		auto* dptr	= dstptr;

		xMemCopy32(dptr, sptr, dest.m_tileSizePix.x);
	}
	return 0;
}

void TextureAtlas::Init(const int2& tileSizePix, int texWidthHint) {
	m_tileSizePix	= tileSizePix;
	m_borderSizePix = 1;
}

void TextureAtlas::AddRows(int numrows) {
	if (numrows <= 0) return;

	auto tileSizeFull = m_tileSizePix + (m_borderSizePix * 2);
	int origBufferLen = m_bufferSizeInTiles.x * m_bufferSizeInTiles.y;

	m_bufferSizeInTiles.y += numrows;

	Bitmap.size = (m_bufferSizeInTiles * tileSizeFull) + (m_borderSizePix * 2);
	Bitmap.buffer.Resize(Bitmap.size.x * Bitmap.size.y * 4);

	Neighbors	= (AtlasNeighborTiles*)xRealloc(Neighbors, sizeof(AtlasNeighborTiles) * m_bufferSizeInTiles.x * m_bufferSizeInTiles.y);
	memset(Neighbors + origBufferLen, 0xff, sizeof(AtlasNeighborTiles) * m_bufferSizeInTiles.x * numrows);
}

int TextureAtlas::AllocTile()
{
	return AllocTiles(1);
}

int TextureAtlas::AllocTiles(int numTiles)
{
	m_Solidified = 0;

	bug_on(m_tileSizePix.isEmpty());
	auto paddingplus  = m_borderSizePix*2;
	auto tileSizeFull = m_tileSizePix + paddingplus;
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
			int unhintedWidth = std::max((tileSizeFull.x * 16) + paddingplus, MinTexWidth);
			if (unhintedWidth > 768 && unhintedWidth < 1024) {
				unhintedWidth = 1024;
			}
			if (unhintedWidth > 2048) {
				unhintedWidth = std::min((tileSizeFull.x * 12) + paddingplus, MaxTexWidth);
			}
			m_bufferSizeInTiles.x = (unhintedWidth-(paddingplus)) / tileSizeFull.x;
		}
		m_bufferSizeInTiles.y = 0;
	}

	auto firstIdx  = m_numPasted;
	m_numPasted   += numTiles;

	while (1) {
		int hasRoomFor = m_bufferSizeInTiles.x * m_bufferSizeInTiles.y;
		int rows       = (numTiles >= (m_bufferSizeInTiles.x / 2)) ? 2 : 1;
		if (m_numPasted < hasRoomFor)  break;
		AddRows(rows);
	}

	return firstIdx;
}

void TextureAtlas::doNeighborCorner(int destid, int neighborid, int2 writepos, int2 readpos)
{
	// get pixel coords of the 'actual' upper-left coord of buffer space
	// readpos/writepos are all either 1 or 0, which when multipled by the tile size, gives us any one of the
	// four corners of a tile.

	auto dstpos = GetTilePosPix(destid) - m_borderSizePix;
	dstpos += (m_tileSizePix + m_borderSizePix) * writepos;

	// src is from inside the tile, so no adjustment by m_borderSizePix is made here:
	if(neighborid < 0) {
		neighborid = destid;
		readpos    = writepos;
	}
	auto srcpos = GetTilePosPix(neighborid);
	srcpos += (m_tileSizePix-1) * readpos;

	bug_on ((srcpos.cmp_any() <= 0) || (srcpos.cmp_any() >= Bitmap.size-1));
	bug_on ((dstpos.cmp_any() <  0) || (dstpos.cmp_any() >= Bitmap.size  ));

	      u32* dst_pixel = Bitmap.GetPtr32() + (dstpos.y * Bitmap.size.x) + dstpos.x;
	const u32* src_pixel = Bitmap.GetPtr32() + (srcpos.y * Bitmap.size.x) + srcpos.x;

	dst_pixel[0] = src_pixel[0];
}

// Fill in horizontal stripe padding above and below a tile.
void TextureAtlas::doNeighborAB(int destid, int neighborid, int writepos, int readpos)
{
	auto dstpos = GetTilePosPix(destid) - int2{ 0, m_borderSizePix };
	dstpos.y += (m_tileSizePix + m_borderSizePix).y * writepos;

	// src is from inside the tile, so no adjustment by m_borderSizePix is made here:

	if(neighborid < 0) {
		neighborid = destid;
		readpos    = writepos;
	}
	auto srcpos = GetTilePosPix(neighborid);
	srcpos.y += (m_tileSizePix.y-1) * readpos;

	bug_on ((srcpos.cmp_any() <= 0) || (srcpos.cmp_any() >= Bitmap.size-1));
	bug_on ((dstpos.cmp_any() <  0) || (dstpos.cmp_any() >= Bitmap.size  ));

	      u32* dst_pixel = Bitmap.GetPtr32() + (dstpos.y * Bitmap.size.x) + dstpos.x;
	const u32* src_pixel = Bitmap.GetPtr32() + (srcpos.y * Bitmap.size.x) + srcpos.x;

	xMemCopy32(dst_pixel, src_pixel, m_tileSizePix.x);
}

void TextureAtlas::doNeighborLR(int destid, int neighborid, int writepos, int readpos)
{
	auto dstpos = GetTilePosPix(destid) - int2{ m_borderSizePix, 0 };
	dstpos.x += (m_tileSizePix + m_borderSizePix).x * writepos;

	// src is from inside the tile, so no adjustment by m_borderSizePix is made here:
	if(neighborid < 0) {
		neighborid = destid;
		readpos	   = writepos;
	}
	auto srcpos = GetTilePosPix(neighborid);
	srcpos.x += (m_tileSizePix.x-1) * readpos;

	bug_on ((srcpos.cmp_any() <= 0) || (srcpos.cmp_any() >= Bitmap.size-1));
	bug_on ((dstpos.cmp_any() <  0) || (dstpos.cmp_any() >= Bitmap.size  ));

	      u32* dst_pixel = Bitmap.GetPtr32() + (dstpos.y * Bitmap.size.x) + dstpos.x;
	const u32* src_pixel = Bitmap.GetPtr32() + (srcpos.y * Bitmap.size.x) + srcpos.x;

	for (int y=0; y<m_tileSizePix.y; ++y, dst_pixel+=Bitmap.size.x, src_pixel+=Bitmap.size.x) {
		dst_pixel[0] = src_pixel[0];
	}
}


void TextureAtlas::Solidify()
{
	if (m_Solidified) return;

	const auto* neighptr = Neighbors;
	const u32*  dest     = (u32*)Bitmap.buffer.GetPtr();
	auto fullTileSize    = m_tileSizePix + m_borderSizePix;
	auto fullTileArea	 = fullTileSize.x*fullTileSize.y;

	auto numTiles = m_numPasted;
	for (int i=0; i<numTiles; ++i) {
		doNeighborCorner(i, Neighbors[i].tileid[1], {1,0}, {1,1});
		doNeighborCorner(i, Neighbors[i].tileid[3], {1,1}, {1,0});
		doNeighborCorner(i, Neighbors[i].tileid[5], {0,1}, {0,0});
		doNeighborCorner(i, Neighbors[i].tileid[7], {0,0}, {0,1});

		doNeighborAB	(i, Neighbors[i].tileid[0], 0, 1);
		doNeighborAB	(i, Neighbors[i].tileid[4], 1, 0);
		doNeighborLR	(i, Neighbors[i].tileid[2], 1, 0);
		doNeighborLR	(i, Neighbors[i].tileid[6], 0, 1);

	}

	m_Solidified = 1;
}