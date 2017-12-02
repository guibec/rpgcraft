
#pragma once

#include "x-ScopedMalloc.h"
#include "x-simd.h"

// xBitmapDataRO - For passing bitmap data into consumer functions.  This provides a more convenient
// lightweight const pointer solution, opposed to the ScopedMalloc managed object in xBitmapData.
struct xBitmapDataRO
{
	const int2			size;
	const u32*			buffer;
};

// Simple is good: Bitmap data is always R8G8B8A8 format
struct xBitmapData
{
	int2				size;
	ScopedMalloc<u8>	buffer;

	operator xBitmapDataRO() const {
		return { size, (u32*)buffer.GetPtr() };
	}
};

union AtlasBorderFlags
{
	struct {
		u8			Left_Neighbor	: 1;
		u8			Left_Self		: 1;
		u8			Right_Neighbor	: 1;
		u8			Right_Self		: 1;
		u8			Bottom_Neighbor	: 1;
		u8			Bottom_Self		: 1;
		u8			Top_Neighbor	: 1;
		u8			Top_Self		: 1;
	};

	u8		b;

	void _static_check() const {
		static_assert(sizeof(AtlasBorderFlags) == sizeof(b), "Ill-formed bitfield union");
	}
};


// TextureAtlas:
//   * arrangements are implementation-defined -- in theory to allow the underlying system to create
//     texture sizes that best suit the hardware.  (long-and-wide vs square textures, etc).
//   * simplicity - atlases are fixed width and just grow in height as needed.
class TextureAtlas
{
public:
	int2				m_tileSizePix		= {};			// size of a tile pasted to the atlas
	int2				m_bufferSizeInTiles	= {};			// dimensions of the atlas, in tiles
	int					m_borderSizePix		= 0;			// border size, for safe bilinear filtering
	int					m_allocWidthHint	= 0;			// hint for more optimal realloc (may be ignored depending on GPU restrictions)
	int					m_initialSize		= 0;			// initial size in tiles
	int					m_numPasted			= 0;			// total number of tiles pasted into the atlas; also serves as index into the next empty slot to paste into.

	AtlasBorderFlags*	m_flags				= nullptr;		// flags for each tile.

public:
	xBitmapData			Bitmap;

public:
	TextureAtlas&		SetBorderSize		(int pix)			{ m_borderSizePix		= pix;				return *this; }
	TextureAtlas&		SetAllocStride		(int texWidthHint)	{ m_allocWidthHint		= texWidthHint;		return *this; }
	TextureAtlas&		SetInitialSize		(int numtiles)		{ m_initialSize			= numtiles;			return *this; }

	void		Init			(const int2& tileSizePix, int texWidthHint = 0);
	void		AddRows			(int numrows);
	int			AllocTile		();
	void		Solidify		();		// fills in all the edges between tiles.


	__ai const u32* GetRawPtr32() const  {
		return (u32*)Bitmap.buffer.GetPtr();
	}

	__ai operator xBitmapDataRO() const {
		return Bitmap;
	}

	__ai int2 GetSizePix() const {
		return Bitmap.size;
	}

	__ai int2 GetTilePosPix(int tileId) const {
		auto fullTileSize = m_tileSizePix + m_borderSizePix;
		return int2 { tileId % m_bufferSizeInTiles.x, tileId / m_bufferSizeInTiles.x } * fullTileSize;
	}
};
