
#include "x-types.h"
#include "x-stl.h"
#include "x-assertion.h"
#include "x-string.h"

#include "x-png-decode.h"
#include "x-gpu-ifc.h"
#include "v-float.h"

#include "ajek-script.h"
#include "UniformMeshes.h"

// Debug font uses a simplified version of tile rendering.
// A draw command is submitted with a grid of character information.
// The character IDs are used to look up the font information from a texture.
//
// DbgFont is divided into two APIs:
//   DbgFont
//     * Renders each character individually using static mesh (XY+UV) and constant buffer (position)
//     * Intended for positional text display, such as XY coords over NPC heads, or similar.
//   DbgFontSheet
//     * Renders a grid of characters using dynamic character map uploaded per-frame.
//     * Intended for use only for two specific usage cases, which are globally defined:
//	       1. universal overlay.
//	       2. drop-down console.
//
// Support list:
//   * Monospaced font only
//   * Scaling and rotation is supported
//   * DbgFont is not read-only during game Render stage - Information can be written
//     to a given font sheet at any time during both Logic and Render pipelines.

static GPU_TextureResource2D	tex_8x8;
static GPU_ShaderVS				s_ShaderVS_DbgFont;
static GPU_ShaderFS				s_ShaderFS_DbgFont;
static GPU_VertexBuffer			s_mesh_anychar;
static GPU_VertexBuffer			s_mesh_worldViewTileID;
static GPU_ConstantBuffer		s_cnstbuf_DbgFontSheet;
static GPU_IndexBuffer			s_idx_UniformQuad;

struct GPU_DbgFontConstants
{
	vFloat2	SrcTexTileSizeUV;
	vInt2	SrcTexSizeInTiles;
	vInt2	CharMapSize;
};

/*{*/ BEGIN_GPU_DATA_STRUCTS	// ---------------------------------------------------

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
};

struct DbgFontDrawItem
{
	int		ppx;		// pixel position x and y
	int		ppy;		// pixel position x and y
	vFloat4	rgba;
	u8		chr;		// character to display
};

#include <vector>

typedef std::vector<DbgFontDrawItem> DbgFontDrawItemContainer;

DbgFontDrawItemContainer		g_dbgFontDrawList;

DbgFontSheet					g_ConsoleSheet;
DbgFontSheet					g_DbgFontOverlay;
bool							s_canRender = false;

void DbgFontSheet::AllocSheet(int2 sizeInPix)
{
	xFree(charmap);
	charmap = nullptr;

	size.x		= sizeInPix.x	/ font.size.x;
	size.y		= sizeInPix.y	/ font.size.y;
	charmap		= (DbgChar*) xMalloc(size.y * size.x * sizeof(DbgChar));
	colormap	= (DbgColor*)xMalloc(size.y * size.x * sizeof(DbgColor));

	dx11_CreateDynamicVertexBuffer(gpu.mesh_charmap, size.y * size.x * sizeof(DbgChar ));
	dx11_CreateDynamicVertexBuffer(gpu.mesh_rgbamap, size.y * size.x * sizeof(DbgColor));
}

struct DbgFontMeshVertex
{
	vFloat2		xy;
	vFloat2		uv;
};

template< typename T >
void table_get_xy(T& dest, LuaTableScope& table)
{
	auto xs = table.get_s32("x");
	auto ys = table.get_s32("y");

	if (xs.isnil()) { xs = table.get_s32(1); }
	if (ys.isnil()) { ys = table.get_s32(2); }

	if (xs.isnil() || ys.isnil()) {
		bug_qa("Invalid table layout for expected Int2 parameter.");
		// Todo: expose a ThrowError that pushes message onto Lua stack and invokes longjmp.
		// Pre-req: must have a single global lua state for all scene magic.
		//ThrowError
	}

	dest.x = xs;
	dest.y = ys;
}

template< typename T >
bool table_get_xy(T& dest, LuaTableScope& table, const char* subtable)
{
	bug_on(!subtable);
	if (auto& subtab = table.get_table(subtable)) {
		table_get_xy(dest, subtab);
		return true;
	}
	return false;
}

void DbgFont_LoadInit(AjekScriptEnv& script)
{
	s_canRender = 0;

	// TODO:
	//  What might be nice here is to build all the known tables and populate them with defaults during
	//  a lua-environment-init step.  Members can be given metadata comments, if desired.  Once populated,
	//  the tables can be displayed using Lua Inspect module (should be included in the repo), eg:
    //
	//     $ print(inspect(DbgConsole))
	//
	//  ... which neatly displays all members supported by the game engine, including metadata comments,
	//  if provided.
	//
	//  Likewise, the console itself can support TAB autocompletion based on table inspection.  Would be
	//  super-cool, right?
	//
	// Drawbacks:
	//   * inspect(table) is not available from external editors.
	//       Workaround: full inspection dump can be written as plaintext or JSON when process starts, and
	//       can be used at a minimum provide a copy/paste template for setting up assignment lists for data-
	//       driven structures.  Advanced integration could include autocomplete -- minimal usefulness for
	//       data-driven things, but higher usefulness for function-driven things.
	//

	const char* dbgConTextureFile	= "..\\8x8font.png";
	g_ConsoleSheet.font.size		= { 8, 8 };
	g_DbgFontOverlay.font.size		= { 8, 8 };
	int2 consoleSizeInPix			= { 1280, 960 };
	int2 overlaySizeInPix			= { 1920, 1080 };

	lua_string consoleShaderFile;
	lua_string consoleShaderEntryVS;
	lua_string consoleShaderEntryFS;

	script.LoadModule("scripts/DbgConsole.lua");

	if (script.HasError()) {
		bug("Unhandled error in DbgFont?");
	}

	if (auto& dbgtable = script.glob_open_table("DbgConsole"))
	{
		dbgConTextureFile = dbgtable.get_string("Texture");
		table_get_xy(g_ConsoleSheet.font.size,	dbgtable, "CharSize");
		table_get_xy(consoleSizeInPix,			dbgtable, "SheetSize");

		if (auto& shadertab = dbgtable.get_table("Shader")) {
			consoleShaderFile		= shadertab.get_string("Filename");
			consoleShaderEntryVS	= shadertab.get_string("VS");
			consoleShaderEntryFS	= shadertab.get_string("FS");

			if (consoleShaderFile.isnil()) {
				if (1)								{ consoleShaderFile		= shadertab.get_string(1); }
				if (consoleShaderEntryVS.isnil())	{ consoleShaderEntryVS	= shadertab.get_string(2); }
				if (consoleShaderEntryFS.isnil())	{ consoleShaderEntryFS	= shadertab.get_string(3); }
			}
		}
	}


	if (1) {
		xBitmapData  pngtex;
		png_LoadFromFile(pngtex, dbgConTextureFile);
		dx11_CreateTexture2D(tex_8x8, pngtex.buffer.GetPtr(), pngtex.width, pngtex.height, GPU_ResourceFmt_R8G8B8A8_UNORM);
	}

	g_ConsoleSheet.font.texture		= &tex_8x8;
	g_ConsoleSheet.AllocSheet(consoleSizeInPix);

	g_DbgFontOverlay.font.texture	= &tex_8x8;
	g_DbgFontOverlay.AllocSheet(overlaySizeInPix);

	dx11_LoadShaderVS(s_ShaderVS_DbgFont, consoleShaderFile, consoleShaderEntryVS);
	dx11_LoadShaderFS(s_ShaderFS_DbgFont, consoleShaderFile, consoleShaderEntryFS);

	dx11_CreateConstantBuffer(s_cnstbuf_DbgFontSheet, sizeof(g_DbgFontOverlay.gpu.consts));
	dx11_CreateIndexBuffer(s_idx_UniformQuad, g_ind_UniformQuad, sizeof(g_ind_UniformQuad));

	dx11_CreateStaticMesh(s_mesh_anychar,	g_mesh_UniformQuad,	sizeof(g_mesh_UniformQuad[0]),	bulkof(g_mesh_UniformQuad));

	s_canRender = 1;
}

void DbgFont_Render()
{
	if (!s_canRender) return;

	// Update dynamic vertex buffers.

	int overlayMeshSize = g_DbgFontOverlay.size.x * g_DbgFontOverlay.size.y;

	for(int i=0; i<overlayMeshSize; ++i) {
		g_DbgFontOverlay.charmap[i] = 'A'; // + (i % 20);
	}

	dx11_UploadDynamicBufferData(g_DbgFontOverlay.gpu.mesh_charmap, g_DbgFontOverlay.charmap,  overlayMeshSize * sizeof(DbgChar ));
	dx11_UploadDynamicBufferData(g_DbgFontOverlay.gpu.mesh_rgbamap, g_DbgFontOverlay.colormap, overlayMeshSize * sizeof(DbgColor));

	g_DbgFontOverlay.gpu.consts.SrcTexTileSizeUV	= vFloat2(1.0f / 128, 1.0f);
	g_DbgFontOverlay.gpu.consts.SrcTexSizeInTiles	= vInt2(128,1);
	g_DbgFontOverlay.gpu.consts.CharMapSize.x		= g_DbgFontOverlay.size.x;
	g_DbgFontOverlay.gpu.consts.CharMapSize.y		= g_DbgFontOverlay.size.y;
	dx11_UpdateConstantBuffer(s_cnstbuf_DbgFontSheet, &g_DbgFontOverlay.gpu.consts);

	// Render!

	dx11_BindShaderVS(s_ShaderVS_DbgFont);
	dx11_BindShaderFS(s_ShaderFS_DbgFont);
	dx11_SetInputLayout(VertexBufferLayout_DbgFont);

//	dx11_SetPrimType(GPU_PRIM_TRIANGLELIST);
	dx11_BindShaderResource(tex_8x8, 0);

	dx11_SetVertexBuffer(s_mesh_anychar,					0, sizeof(g_mesh_UniformQuad[0]), 0);
	dx11_SetVertexBuffer(g_DbgFontOverlay.gpu.mesh_charmap,	1, sizeof(DbgChar),  0);
	dx11_SetVertexBuffer(g_DbgFontOverlay.gpu.mesh_rgbamap,	2, sizeof(DbgColor), 0);

	//dx11_SetVertexBuffer(g_mesh_worldViewColor, 2, sizeof(g_ViewUV[0]), 0);

	dx11_BindConstantBuffer(s_cnstbuf_DbgFontSheet, 0);
	dx11_SetIndexBuffer(s_idx_UniformQuad, 16, 0);
	dx11_DrawIndexedInstanced(6, overlayMeshSize, 0, 0, 0);

}
