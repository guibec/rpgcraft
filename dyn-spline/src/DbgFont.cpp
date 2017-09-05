
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

typedef u8	DbgChar;

union DbgColor
{
	struct {
		u8		a,b,g,r;
	};

	u32		rgba;

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

	dx11_CreateDynamicVertexBuffer(gpu.mesh_charmap, size.y * size.x);
	dx11_CreateDynamicVertexBuffer(gpu.mesh_rgbamap, size.y * size.x);
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
			consoleShaderEntryVS	= shadertab.get_string("CallVS");
			consoleShaderEntryFS	= shadertab.get_string("CallFS");

			if (consoleShaderFile.isnil()) {
				if (1)								{ consoleShaderFile		= shadertab.get_string(1); }
				if (consoleShaderEntryVS.isnil())	{ consoleShaderEntryVS	= shadertab.get_string(2); }
				if (consoleShaderEntryFS.isnil())	{ consoleShaderEntryFS	= shadertab.get_string(3); }
			}
		}
	}



//	auto woot = DbgConPkg.get_table("font");

	if (1) {
		xBitmapData  pngtex;
		png_LoadFromFile(pngtex, dbgConTextureFile);
		dx11_CreateTexture2D(tex_8x8, pngtex.buffer.GetPtr(), pngtex.width, pngtex.height, GPU_ResourceFmt_R8G8B8A8_UNORM);
	}

	g_ConsoleSheet.font.texture		= &tex_8x8;
	g_ConsoleSheet.AllocSheet(consoleSizeInPix);

	g_DbgFontOverlay.font.texture	= &tex_8x8;
	g_DbgFontOverlay.AllocSheet(overlaySizeInPix);

	dx11_LoadShaderVS(s_ShaderVS_DbgFont, "DbgFont.fx", "VS");
	dx11_LoadShaderFS(s_ShaderFS_DbgFont, "DbgFont.fx", "PS");
	
	dx11_CreateConstantBuffer(s_cnstbuf_DbgFontSheet, sizeof(g_DbgFontOverlay.gpu.consts));
	dx11_CreateIndexBuffer(s_idx_UniformQuad, g_ind_UniformQuad, sizeof(g_ind_UniformQuad));

	s_canRender = 1;
}

void DbgFont_Render()
{
	if (!s_canRender) return;

	// Update dynamic vertex buffers.

	int overlayMeshSize = g_DbgFontOverlay.size.x * g_DbgFontOverlay.size.y;

	dx11_UploadDynamicBufferData(g_DbgFontOverlay.gpu.mesh_charmap, g_DbgFontOverlay.charmap,  overlayMeshSize * sizeof(DbgChar ));
	dx11_UploadDynamicBufferData(g_DbgFontOverlay.gpu.mesh_rgbamap, g_DbgFontOverlay.colormap, overlayMeshSize * sizeof(DbgColor));

	// Render!

	g_DbgFontOverlay.gpu.consts.SrcTexTileSizeUV	= vFloat2(1.0f / 128, 1.0f);
	g_DbgFontOverlay.gpu.consts.CharMapSize.x		= g_DbgFontOverlay.size.x;
	g_DbgFontOverlay.gpu.consts.CharMapSize.y		= g_DbgFontOverlay.size.y;

	dx11_BindShaderVS(s_ShaderVS_DbgFont);
	dx11_BindShaderFS(s_ShaderFS_DbgFont);
	dx11_SetInputLayout(VertexBufferLayout_TileMap);

//	dx11_SetPrimType(GPU_PRIM_TRIANGLELIST);
	dx11_BindShaderResource(tex_8x8, 0);

	dx11_SetVertexBuffer(s_mesh_anychar,					0, sizeof(g_mesh_UniformQuad[0]), 0);
	dx11_SetVertexBuffer(g_DbgFontOverlay.gpu.mesh_charmap,	1, sizeof(DbgChar),  0);
	dx11_SetVertexBuffer(g_DbgFontOverlay.gpu.mesh_rgbamap,	1, sizeof(DbgColor), 0);

	//dx11_SetVertexBuffer(g_mesh_worldViewColor, 2, sizeof(g_ViewUV[0]), 0);

	dx11_UpdateConstantBuffer(s_cnstbuf_DbgFontSheet, &g_DbgFontOverlay.gpu.consts);
	dx11_BindConstantBuffer(s_cnstbuf_DbgFontSheet, 0);
	dx11_SetIndexBuffer(s_idx_UniformQuad, 16, 0);
	dx11_DrawIndexedInstanced(6, overlayMeshSize, 0, 0, 0);

}
