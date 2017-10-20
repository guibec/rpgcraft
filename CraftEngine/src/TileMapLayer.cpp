
#include "PCH-rpgcraft.h"
#include "x-gpu-ifc.h"
#include "v-float.h"
#include "x-png-decode.h"

#include "ajek-script.h"
#include "Scene.h"

#include "TileMapLayer.h"
#include "DbgFont.h"

DECLARE_MODULE_NAME("TileMap");

// Probably need some sort of classification system here.
// Some terrains may change over time, such as grow moss after being crafted.
//  - Maybe better handled as a generic "age" engine feature?
//  - But there could be different types of mosses, or stalagmites, or other environment changes.

struct TerrainMapItem {
	int		tilesetId;		// specific tile from the set is determined according to surrounding tiles at render time.
};

TerrainMapItem*		g_WorldMap		= nullptr;
u32*				g_ViewTileID	= nullptr;	// temporarily global - will localize later.

// Probably in tile coordinates with fractional being partial-tile position
float g_playerX;
float g_playerY;


static const int TerrainTileW = 256;
static const int TerrainTileH = 256;

int g_setCountX = 0;
int g_setCountY = 0;

GPU_ConstantBuffer		g_cnstbuf_TileMap;

TileMapLayer::TileMapLayer() {
}

void TileMapLayer::SceneInit(const char* script_objname)
{
	if (!script_objname) {
		script_objname = Entity_LookupName(m_gid);
	}

	auto& script = g_scriptEnv;

	// default test values in case script loading is bypassed.
	ViewMeshSizeX = 24;
	ViewMeshSizeY = 24;
	ViewInstanceCount	= ViewMeshSizeY * ViewMeshSizeX;
	ViewVerticiesCount	= ViewInstanceCount * 6;


	if (auto& worldViewTab = script.glob_open_table(script_objname))
	{
		auto tileSheetFilename	= worldViewTab.get_string("TileSheet");

		if (auto getMeshSize = worldViewTab.push_func("getMeshSize")) {
			getMeshSize.pusharg("desktop");
			getMeshSize.pusharg(1920.0f);
			getMeshSize.pusharg(1080.0f);
			getMeshSize.execcall(2);		// 2 - num return values
			float SizeX = getMeshSize.getresult<float>();
			float SizeY = getMeshSize.getresult<float>();

			ViewMeshSizeX = int(std::ceilf(SizeX / TileSizeX));
			ViewMeshSizeY = int(std::ceilf(SizeY / TileSizeY));

			ViewMeshSizeX = std::min(ViewMeshSizeX, WorldSizeX);
			ViewMeshSizeY = std::min(ViewMeshSizeY, WorldSizeY);
			ViewInstanceCount  = ViewMeshSizeY * ViewMeshSizeX;
			ViewVerticiesCount = ViewInstanceCount * 6;
		}
	}

	if (1) {
		xBitmapData  pngtex;
		png_LoadFromFile(pngtex, ".\\rpg_maker_vx__modernrtp_tilea2_by_painhurt-d3f7rwg.png");
		dx11_CreateTexture2D(gpu.tex_floor, pngtex.buffer.GetPtr(), pngtex.width, pngtex.height, GPU_ResourceFmt_R8G8B8A8_UNORM);

		// Assume pngtex is rpgmaker layout for now.

		g_setCountX = pngtex.width	/ 64;
		g_setCountY = pngtex.height	/ (64 + 32);
	}

	WorldMap_Procgen();
	PopulateUVs({0,0});

	// GPU Resource Initialization.

	xMemZero(gpu.layout_tilemap);
	gpu.layout_tilemap.AddVertexSlot( {
		{ "POSITION", GPU_ResourceFmt_R32G32_FLOAT	},
		{ "TEXCOORD", GPU_ResourceFmt_R32G32_FLOAT	}
	});

	gpu.layout_tilemap.AddInstanceSlot( {
		{ "TileID", GPU_ResourceFmt_R32_UINT }
	});

	gpu.layout_tilemap.AddInstanceSlot( {
		{ "COLOR",  GPU_ResourceFmt_R32G32B32A32_FLOAT }
	});

	// TODO: only want to initialize this once for all tilemap instances.
	dx11_CreateConstantBuffer(g_cnstbuf_TileMap,	sizeof(GPU_TileMapConstants));

	dx11_CreateStaticMesh(gpu.mesh_tile,				g_mesh_UniformQuad,	sizeof(g_mesh_UniformQuad[0]),	bulkof(g_mesh_UniformQuad));
	//dx11_CreateStaticMesh(gpu.mesh_worldViewTileID,		g_ViewTileID,		sizeof(g_ViewTileID[0]),		ViewInstanceCount);
	dx11_CreateDynamicVertexBuffer(gpu.mesh_worldViewTileID, sizeof(g_ViewTileID[0]) * ViewInstanceCount);

	dx11_LoadShaderVS(g_ShaderVS_Tiler, "TileMap.fx", "VS");
	dx11_LoadShaderFS(g_ShaderFS_Tiler, "TileMap.fx", "PS");
}

void WorldMap_Procgen()
{
	g_WorldMap	= (TerrainMapItem*)	xRealloc(g_WorldMap, WorldSizeX    * WorldSizeY    * sizeof(TerrainMapItem));

	g_playerX = 0;
	g_playerY = 0;

	// Fill map with boring grass.

	for (int y=0; y<WorldSizeY; ++y) {
		for (int x=0; x<WorldSizeX; ++x) {
			g_WorldMap[(y * WorldSizeX) + x].tilesetId = 11;
		}
	}

	// Write a border around the entire map for now.

	for (int x=0; x<WorldSizeX; ++x) {
		g_WorldMap[((     0	     ) * WorldSizeX) + x].tilesetId = 21;
		g_WorldMap[((     1	     ) * WorldSizeX) + x].tilesetId = 22;
		g_WorldMap[((WorldSizeY-1) * WorldSizeX) + x].tilesetId = 22;
		g_WorldMap[((WorldSizeY-2) * WorldSizeX) + x].tilesetId = 21;
	}

	for (int y=0; y<WorldSizeY; ++y) {
		g_WorldMap[(y * WorldSizeX) + 0				].tilesetId = 21;
		g_WorldMap[(y * WorldSizeX) + 1				].tilesetId = 21;
		g_WorldMap[(y * WorldSizeX) + (WorldSizeX-2)].tilesetId = 21;
		g_WorldMap[(y * WorldSizeX) + (WorldSizeX-1)].tilesetId = 21;
	}
}

void TileMapLayer::PopulateUVs(const int2& viewport_offset)
{
	g_ViewTileID = (u32*)xRealloc(g_ViewTileID, ViewInstanceCount * sizeof(u32));

	// Populate view mesh according to world map information:

	vFloat2 incr_set_uv = vFloat2(1.0f / g_setCountX, 1.0f / g_setCountY);
	vFloat2 t16uv = incr_set_uv / vFloat2(2.0f, 3.0f);

	for (int yl=0; yl<ViewMeshSizeY; ++yl) {
		for (int xl=0; xl<ViewMeshSizeX; ++xl) {
			int y = yl + viewport_offset.y;
			int x = xl + viewport_offset.x;
			int instanceId	= ((yl*ViewMeshSizeX) + xl);
			int vertexId	= instanceId * 6;

			if (y<0 || x<0)						{ g_ViewTileID[instanceId] = 12; continue; }
			if (y>=WorldSizeY || x>=WorldSizeX) { g_ViewTileID[instanceId] = 12; continue; }

			int setId		= g_WorldMap[(y * WorldSizeX) + x].tilesetId;
			int setX		= setId % g_setCountX;
			int setY		= setId / g_setCountX;

			// Look at surrounding tiles to decide how to match this tile...
			// TODO: Try moving this into Lua?  As a proof-of-concept for rapid iteration?
			//    Or is this not appropriate scope for; scripting yet?  Hmm!

			bool match_above1 = false;
			bool match_below1 = false;
			bool match_left1  = false;
			bool match_right1 = false;

			if (y > 0) {
				int idx = ((y-1) * WorldSizeX) + (x+0);
				match_above1 = (g_WorldMap[idx].tilesetId == setId);
			}

			if (y < WorldSizeY-1) {
				int idx = ((y+1) * WorldSizeX) + (x+0);
				match_below1 = (g_WorldMap[idx].tilesetId == setId);
			}

			if (x > 0) {
				int idx  = ((y+0) * WorldSizeX) + (x-1);
				match_left1 = (g_WorldMap[idx].tilesetId == setId);
			}

			if (x < WorldSizeX-1) {
				int idx = ((y+0) * WorldSizeX) + (x+1);
				match_right1 = (g_WorldMap[idx].tilesetId == setId);
			}

			int subTileX = 0;
			int subTileY = 0;

			if (match_above1 && match_below1) {
				if (!match_left1 || !match_right1) {
					//subTileX = 0;
					//subTileY = 1;
				}
			}

			if (match_left1 && match_right1) {
				//subTileX = 1;
				if (match_above1) {
					//subTileY = 4;
				}
			}

			//subTileY += 3;

			g_ViewTileID[instanceId]  = (setY * g_setCountX) + setX;
			g_ViewTileID[instanceId] += (subTileY * 4) + subTileX;
		}
	}

	// Sort various sprite batches
}

void TileMapLayer::Tick() {
	// determine tile map draw position according to camera position.

	gpu.consts.TileAlignedDisp		= vFloat2(floorf(g_ViewCamera.m_Eye.x), floorf(-g_ViewCamera.m_Eye.y));
	gpu.consts.SrcTexSizeInTiles	= vInt2(g_setCountX, g_setCountY);
	gpu.consts.SrcTexTileSizeUV		= vFloat2(1.0f / g_setCountX, 1.0f / g_setCountY) / vFloat2(2.0f, 3.0f);
	gpu.consts.TileMapSizeX			= ViewMeshSizeX;
	gpu.consts.TileMapSizeY			= ViewMeshSizeY;

	auto disp = int2 { (int)gpu.consts.TileAlignedDisp.x, (int)gpu.consts.TileAlignedDisp.y  };
	disp.x -= ViewMeshSizeX / 2;
	disp.y -= ViewMeshSizeY / 2;

	PopulateUVs(disp);
	dx11_UploadDynamicBufferData(gpu.mesh_worldViewTileID, g_ViewTileID,  sizeof(g_ViewTileID[0]) * ViewInstanceCount);
}


void TileMapLayer::Draw() const
{
	dx11_BindShaderVS(g_ShaderVS_Tiler);
	dx11_BindShaderFS(g_ShaderFS_Tiler);
	dx11_SetInputLayout(gpu.layout_tilemap);

//	dx11_SetPrimType(GPU_PRIM_TRIANGLELIST);
	dx11_BindShaderResource(gpu.tex_floor, 0);

	dx11_SetVertexBuffer(gpu.mesh_tile,				0, sizeof(g_mesh_UniformQuad[0]), 0);
	dx11_SetVertexBuffer(gpu.mesh_worldViewTileID,	1, sizeof(g_ViewTileID[0]), 0);
	//dx11_SetVertexBuffer(g_mesh_worldViewColor, 2, sizeof(g_ViewUV[0]), 0);

	dx11_UpdateConstantBuffer(g_cnstbuf_TileMap, &gpu.consts);
	dx11_BindConstantBuffer(g_cnstbuf_TileMap, 1);
	dx11_SetIndexBuffer(g_idx_box2D, 16, 0);
	dx11_DrawIndexedInstanced(6, ViewInstanceCount, 0, 0, 0);

}
