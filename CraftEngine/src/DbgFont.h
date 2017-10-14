#pragma once

#include "x-types.h"
#include "v-float.h"
#include "x-gpu-ifc.h"

#include "x-ForwardDefs.h"

/*{*/ BEGIN_GPU_DATA_STRUCTS	// ---------------------------------------------------

struct GPU_DbgFontConstants
{
	vFloat2		SrcTexTileSizeUV;
	vInt2		SrcTexSizeInTiles;
	vInt2		CharMapSize;
	vFloat2		ProjectionXY;
	vFloat2		ProjectionScale;
};

// Notes:
//  * In order to use u8 character information, the inputs to the shader must be provided as
//    textures rather than Vertex Buffers.  This is needed because vertex buffers have buffer
//    stride and alignment limitations.  I've had issues getting texture.Load() to work correctly,
//    so for now using space-inefficient 32-bit vertex buffer formats.

typedef u32	DbgChar;

union DbgColor
{
	struct {
		float		a,b,g,r;
	};

	u128		rgba;

	// rgba - could be changed to a u8 clut into a color table, if performance of dbgfont
	// is a problem later on.  (seems unlikely)
} __packed;

/*}*/ END_GPU_DATA_STRUCTS		// ---------------------------------------------------

struct DbgFontSheet
{
	int2	size;

	DbgChar*	charmap;
	DbgColor*	colormap;

	struct {
		GPU_DbgFontConstants	consts;
		GPU_DynVsBuffer			mesh_charmap;
		GPU_DynVsBuffer			mesh_rgbamap;
	} gpu;

	struct {
		int2	size;
		const GPU_TextureResource2D*	texture;
	} font;

	void		AllocSheet		(int2 sizeInPix);
	void		SceneBegin		();
	void		Write			(int x, int y, const xString& msg);
};

struct DbgFontDrawItem
{
	int		ppx;		// pixel position x and y
	int		ppy;		// pixel position x and y
	vFloat4	rgba;
	u8		chr;		// character to display
};

extern void DbgFont_LoadInit		();
extern void DbgFont_SceneBegin		();
extern void DbgFont_SceneRender		();
