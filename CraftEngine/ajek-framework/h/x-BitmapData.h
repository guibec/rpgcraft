
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

			u32*	GetPtr32()			{ return (u32*)buffer.GetPtr(); }
	const	u32*	GetPtr32() const	{ return (u32*)buffer.GetPtr(); }
};

struct AtlasNeighborTiles
{
	// eight neighbor tile IDs, starting at high noon and going clockwise around the tile.
	// Negative value indicates "no neighbor" -- edges will be copied from self tile.
	int		tileid[8];
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
	bool				m_Solidified		= 0;


public:
	AtlasNeighborTiles*	Neighbors			= nullptr;		// neighboring tile information for each tile.
	xBitmapData			Bitmap;

protected:
	void				doNeighborCorner	(int destid, int neighborid, int2 writepos, int2 readpos);
	void				doNeighborAB		(int destid, int neighborid, int writepos, int readpos);					// above / below neighbors
	void				doNeighborLR		(int destid, int neighborid, int writepos, int readpos);					// left  / right neighbors

public:
	TextureAtlas&		SetBorderSize		(int pix)			{ m_borderSizePix		= pix;				return *this; }
	TextureAtlas&		SetAllocStride		(int texWidthHint)	{ m_allocWidthHint		= texWidthHint;		return *this; }
	TextureAtlas&		SetInitialSize		(int numtiles)		{ m_initialSize			= numtiles;			return *this; }

	void		Init			(const int2& tileSizePix, int texWidthHint = 0);
	void		AddRows			(int numrows);
	int			AllocTile		();
	int			AllocTiles		(int numTiles);
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
		auto fullTileSize = m_tileSizePix + (m_borderSizePix*2);
		return (int2 { tileId % m_bufferSizeInTiles.x, tileId / m_bufferSizeInTiles.x } * fullTileSize) + m_borderSizePix;
	}
};
