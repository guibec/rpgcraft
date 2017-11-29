#pragma once

#include "x-BitmapData.h"

struct rgba32 {
	u8	 a,b,g,r;
	rgba32(u8 r_, u8 g_, u8 b_, u8 a_) {
		a = a_;
		b = b_;
		g = g_;
		r = r_;
	}
	rgba32(u32 im_code) {
		// im_code expected format is ARGB, which is convenient for annotation, but needs to be converted
		// into machine code for both big and little endians.

		a = (im_code >> 24) & 0xff;
		b = (im_code >>  0) & 0xff;
		g = (im_code >>  8) & 0xff;
		r = (im_code >> 16) & 0xff;

	}
	u32 w32() const {
		return (u32&)(*this);
	}
};

// TextureAtlas:
//   * arrangements are implementation-defined -- in theory to allow the underlying system to create
//     texture sizes that best suit the hardware.  (long-and-wide vs square textures, etc).
//   * simplicity - atlases are fixed width and just grow in height as needed.

struct TextureAtlas
{
	int2				m_tileSizePix		= {};			// size of a tile pasted to the atlas
	int2				m_bufferSizeInTiles	= {};			// dimensions of the atlas, in tiles
	int					m_borderSizePix		= 0;			// border size, for safe bilinear filtering
	int					m_allocWidthHint	= 0;			// hint for more optimal realloc (may be ignored depending on GPU restrictions)
	int					m_initialSize		= 0;			// initial size in tiles

	int					m_numPasted			= 0;			// total number of tiles pasted into the atlas; also serves as index into the next empty slot to paste into.
	ScopedMalloc<u8>	m_buffer;

	TextureAtlas&		SetBorderSize		(int pix)			{ m_borderSizePix		= pix;				return *this; }
	TextureAtlas&		SetAllocStride		(int texWidthHint)	{ m_allocWidthHint		= texWidthHint;		return *this; }
	TextureAtlas&		SetInitialSize		(int numtiles)		{ m_initialSize			= numtiles;			return *this; }

	void		Init			(const int2& tileSizePix, int texWidthHint = 0);
	void		AddRows			(int numrows);
	int			AllocTile		();

	__ai int2 GetTilePosPix(int tileId) const {
		return { tileId % m_bufferSizeInTiles.x, tileId / m_bufferSizeInTiles.x };
	}
};

namespace imgtool
{
	static const u32 mask_alpha = 0x000000ff;
	static const u32 mask_rgb   = 0xffffff00;

	void ConvertOpaqueColorToAlpha(xBitmapData& image, const rgba32& color);
	void CutTex(xBitmapData& dest, const xBitmapData& src, int2 xy1, int2 xy2);
	void CutTex_and_ConvertOpaqueColorToAlpha(xBitmapData& dest, const xBitmapData& src, int2 xy1, int2 xy2, const rgba32& color);
	int AddTileToAtlas(TextureAtlas& dest, xBitmapData& src, int gpu_border=1);
};

