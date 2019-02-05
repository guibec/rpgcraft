#include "PCH-rpgcraft.h"

#include "x-types.h"
#include "x-stl.h"
#include "x-assertion.h"
#include "x-string.h"

#include "x-png-decode.h"
#include "x-gpu-ifc.h"
#include "v-float.h"

#include "imgtools.h"
#include "UniformMeshes.h"
#include "DbgTextOverlay.h"
#include "Scene.h"

#include "ajek-script.h"

#include <vector>

#include <DirectXMath.h>

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
//         1. universal overlay.
//         2. drop-down console.
//
// Support list:
//   * Monospaced font only
//   * Scaling and rotation is supported
//   * DbgFont is not read-only during game Render stage - Information can be written
//     to a given font sheet at any time during both Logic and Render pipelines.


static GPU_TextureResource2D    tex_6x8;
static GPU_ShaderVS             s_ShaderVS_DbgFont;
static GPU_ShaderFS             s_ShaderFS_DbgFont;
static GPU_VertexBuffer         s_mesh_anychar;
static GPU_VertexBuffer         s_mesh_worldViewTileID;
static GPU_ConstantBuffer       s_cnstbuf_Projection;
static GPU_ConstantBuffer       s_cnstbuf_DbgFontSheet;
static GPU_IndexBuffer          s_idx_UniformQuad;

static GPU_ViewCameraConsts     m_ViewConsts;

typedef std::vector<DbgFontDrawItem> DbgFontDrawItemContainer;

DbgFontDrawItemContainer        g_dbgFontDrawList;

DbgFontSheet                    g_ConsoleSheet;
DbgFontSheet                    g_DbgTextOverlay;
bool                            s_canRender = false;

void DbgFontSheet::AllocSheet(int2 sizeInPix)
{
    size.x      = sizeInPix.x   / font.size.x;
    size.y      = sizeInPix.y   / font.size.y;
    charmap     = (DbgChar*) xRealloc(charmap,  size.y * size.x * sizeof(DbgChar));
    colormap    = (DbgColor*)xRealloc(colormap, size.y * size.x * sizeof(DbgColor));

    dx11_CreateDynamicVertexBuffer(gpu.mesh_charmap, size.y * size.x * sizeof(DbgChar ));
    dx11_CreateDynamicVertexBuffer(gpu.mesh_rgbamap, size.y * size.x * sizeof(DbgColor));
}

template< typename T >
void xMemSetObjs(T* dest, const T& data, int sizeInInstances)
{
    for (int i=0; i<sizeInInstances; ++i) {
        dest[i] = data;
    }
}

void DbgFontSheet::SceneLogic()
{
    xMemSetObjs (charmap,   { 0 },                          size.y * size.x);
    xMemSetObjs (colormap, {{ 1.0f, 0.5f, 0.5f, 0.5f }},    size.y * size.x);
}

void DbgFontSheet::Write(int x, int y, const xString& msg)
{
    int pos = (y * size.x) + x;
    for (int i=0; i<msg.GetLength(); ++i) {
        g_DbgTextOverlay.charmap[pos+i] = (u8)msg[i];
    }
}


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

static GPU_InputDesc InputLayout_DbgFont;

void DbgFont_MakeVertexLayout()
{
    InputLayout_DbgFont.Reset();
    InputLayout_DbgFont.AddVertexSlot( {
        { "POSITION", GPU_ResourceFmt_R32G32_FLOAT  },
        { "TEXCOORD", GPU_ResourceFmt_R32G32_FLOAT  }
    });

    InputLayout_DbgFont.AddInstanceSlot( {
        { "TileID", GPU_ResourceFmt_R32_UINT }
    });

    InputLayout_DbgFont.AddInstanceSlot( {
        { "COLOR",  GPU_ResourceFmt_R32G32B32A32_FLOAT }
    });
}

void DbgTextOverlay_LoadInit()
{
    auto& script = g_scriptEnv;

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

    float2 edgeOffset        = float2 { 4, 4 };
    float2 backbuffer_size   = (g_client_size_pix - edgeOffset);

    // Provide a fixed-size font grid that scales "roughly" to match the backbuffer resolution.
    // use floorf to ensure the scalar is even-numbered, to avoid uglified font syndrome.
    // (note: in theory above 3x scale it's probably ok to be fractional, as it won't really look bad anyway...)

    auto scalarxy = floorf(g_client_size_pix / float2 { 640, 360 });
    auto scalar = std::max(scalarxy.x, scalarxy.y);

    edgeOffset       /= scalar;
    backbuffer_size  /= scalar;

    g_ConsoleSheet.font.size        = { 6, 8 };
    g_DbgTextOverlay.font.size      = { 6, 8 };
    auto consoleSizeInPix           = (int2)floorf(g_client_size_pix / 2    );
    auto overlaySizeInPix           = (int2)floorf(backbuffer_size);

    lua_string consoleShaderFile;
    lua_string consoleShaderEntryVS;
    lua_string consoleShaderEntryFS;

    script.LoadModule("scripts/DbgConsole.lua");

    if (script.HasError()) {
        bug("Unhandled error in DbgFont?");
    }

    if (auto& dbgtable = script.glob_open_table("DbgConsole"))
    {
        table_get_xy(g_ConsoleSheet.font.size,  dbgtable, "CharSize");
        table_get_xy(consoleSizeInPix,          dbgtable, "SheetSize");

        if (auto& shadertab = dbgtable.get_table("Shader")) {
            consoleShaderFile       = shadertab.get_string("Filename");
            consoleShaderEntryVS    = shadertab.get_string("VS");
            consoleShaderEntryFS    = shadertab.get_string("FS");

            if (consoleShaderFile.isnil()) {
                if (1)                              { consoleShaderFile     = shadertab.get_string(1); }
                if (consoleShaderEntryVS.isnil())   { consoleShaderEntryVS  = shadertab.get_string(2); }
                if (consoleShaderEntryFS.isnil())   { consoleShaderEntryFS  = shadertab.get_string(3); }
            }
        }
    }


    if (1) {
        xBitmapData  pngtex;
        DbgFont::BlitAtlasTexture(pngtex);
        dx11_CreateTexture2D(tex_6x8, pngtex.buffer.GetPtr(), pngtex.size, GPU_ResourceFmt_R8G8B8A8_UNORM);
    }

    g_ConsoleSheet.font.texture     = &tex_6x8;
    g_ConsoleSheet.AllocSheet(consoleSizeInPix);

    g_DbgTextOverlay.font.texture   = &tex_6x8;
    g_DbgTextOverlay.AllocSheet(overlaySizeInPix);

    dx11_LoadShaderVS(s_ShaderVS_DbgFont, consoleShaderFile, consoleShaderEntryVS);
    dx11_LoadShaderFS(s_ShaderFS_DbgFont, consoleShaderFile, consoleShaderEntryFS);

    dx11_CreateConstantBuffer(s_cnstbuf_Projection,     sizeof(m_ViewConsts));
    dx11_CreateConstantBuffer(s_cnstbuf_DbgFontSheet,   sizeof(g_DbgTextOverlay.gpu.consts));
    dx11_CreateIndexBuffer(s_idx_UniformQuad, g_ind_UniformQuad, sizeof(g_ind_UniformQuad));

    dx11_CreateStaticMesh(s_mesh_anychar,   g_mesh_UniformQuad, sizeof(g_mesh_UniformQuad[0]),  bulkof(g_mesh_UniformQuad));

    u128    m_Eye;
    u128    m_At;
    u128    m_Up;           // X is angle.  Y is just +/- (orientation)? Z is unused?

    //m_Eye = XMVectorSet( overlaySizeInPix.x/2, overlaySizeInPix.y/2, -6.0f, 0.0f );
    //m_At  = XMVectorSet( overlaySizeInPix.x/2, overlaySizeInPix.y/2,  0.0f, 0.0f );

    m_Eye   = XMVectorSet( 0.0f, 0.0f, -6.0f, 0.0f );
    m_At    = XMVectorSet( 0.0f, 0.0f,  0.0f, 0.0f );
    m_Up    = XMVectorSet( 0.0f, 1.0f,  0.0f, 0.0f );

    m_ViewConsts.View       = XMMatrixLookAtLH(m_Eye, m_At, m_Up);
    m_ViewConsts.Projection = XMMatrixOrthographicLH(overlaySizeInPix.x, overlaySizeInPix.y, 0.0001f, 1000.0f);
    m_ViewConsts.Projection = XMMatrixOrthographicOffCenterLH(-edgeOffset.x, backbuffer_size.x, backbuffer_size.y, -edgeOffset.y, 0.0001f, 1000.0f);

    s_canRender = 1;
}

void DbgTextOverlay_NewFrame()
{
    g_DbgTextOverlay    .SceneLogic();
    g_ConsoleSheet      .SceneLogic();
}

void DbgTextOverlay_SceneRender()
{
    if (!s_canRender) return;

    // Update dynamic vertex buffers.

    int overlayMeshSize = g_DbgTextOverlay.size.x * g_DbgTextOverlay.size.y;

    g_DbgTextOverlay.Write(0,0, "RPGCraft Version 2018-01-01.BuildNumber");

    dx11_UploadDynamicBufferData(g_DbgTextOverlay.gpu.mesh_charmap, g_DbgTextOverlay.charmap,  overlayMeshSize * sizeof(DbgChar ));
    dx11_UploadDynamicBufferData(g_DbgTextOverlay.gpu.mesh_rgbamap, g_DbgTextOverlay.colormap, overlayMeshSize * sizeof(DbgColor));

    g_DbgTextOverlay.gpu.consts.SrcTexTileSizeUV    = vFloat2(1.0f / DbgFont::CharacterCodeCount, 1.0f);
    g_DbgTextOverlay.gpu.consts.SrcTexSizeInTiles   = vInt2(DbgFont::CharacterCodeCount,1);
    g_DbgTextOverlay.gpu.consts.CharMapSize.x       = g_DbgTextOverlay.size.x;
    g_DbgTextOverlay.gpu.consts.CharMapSize.y       = g_DbgTextOverlay.size.y;
    g_DbgTextOverlay.gpu.consts.TileSize            = g_DbgTextOverlay.font.size;
    //g_DbgTextOverlay.gpu.consts.ProjectionXY      = vFloat2(0.0f, 0.0f);
    //g_DbgTextOverlay.gpu.consts.ProjectionScale       = vFloat2(1.0f, 1.0f);

    GPU_ViewCameraConsts    viewConsts;
    viewConsts.View         = XMMatrixTranspose(m_ViewConsts.View);
    viewConsts.Projection   = XMMatrixTranspose(m_ViewConsts.Projection);

    dx11_UpdateConstantBuffer(s_cnstbuf_Projection,     &viewConsts);
    dx11_UpdateConstantBuffer(s_cnstbuf_DbgFontSheet,   &g_DbgTextOverlay.gpu.consts);

    // Render!

    DbgFont_MakeVertexLayout();
    dx11_SetInputLayout(InputLayout_DbgFont);

    dx11_BindShaderVS(s_ShaderVS_DbgFont);
    dx11_BindShaderFS(s_ShaderFS_DbgFont);

    dx11_BindShaderResource(tex_6x8, 0);

    dx11_SetVertexBuffer(s_mesh_anychar,                    0, sizeof(g_mesh_UniformQuad[0]), 0);
    dx11_SetVertexBuffer(g_DbgTextOverlay.gpu.mesh_charmap, 1, sizeof(DbgChar),  0);
    dx11_SetVertexBuffer(g_DbgTextOverlay.gpu.mesh_rgbamap, 2, sizeof(DbgColor), 0);

    //dx11_SetVertexBuffer(g_mesh_worldViewColor, 2, sizeof(g_ViewUV[0]), 0);

    dx11_BindConstantBuffer(s_cnstbuf_Projection,   0);
    dx11_BindConstantBuffer(s_cnstbuf_DbgFontSheet, 1);
    dx11_SetIndexBuffer(s_idx_UniformQuad, 16, 0);
    dx11_SetPrimType(GPU_PRIM_TRIANGLELIST);
    dx11_DrawIndexedInstanced(6, overlayMeshSize, 0, 0, 0);

}
