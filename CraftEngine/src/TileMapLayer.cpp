
#include "PCH-rpgcraft.h"
#include "x-gpu-ifc.h"
#include "v-float.h"

#include "ajek-script.h"
#include "Scene.h"

#include "TileMapLayer.h"
#include "DbgFont.h"

DECLARE_MODULE_NAME("TileMap");

// Probably need some sort of classification system here.
// Some terrains may change over time, such as grow moss after being crafted.
//  - Maybe better handled as a generic "age" engine feature?
//  - But there could be different types of mosses, or stalagmites, or other environment changes.

u32*				g_ViewTileID	= nullptr;	// temporarily global - will localize later.

static const int TerrainTileW = 256;
static const int TerrainTileH = 256;

GPU_ConstantBuffer		g_cnstbuf_TileMap;



TileMapLayer::TileMapLayer() {
}

void TileMapLayer::PopulateUVs(const TerrainMapItem* terrain, const int2& viewport_offset)
{
	bug_on(!terrain);
	g_ViewTileID = (u32*)xRealloc(g_ViewTileID, ViewInstanceCount * sizeof(u32));

	// Populate view mesh according to world map information:

	vFloat2 incr_set_uv = vFloat2(1.0f / m_setCount);
	vFloat2 t16uv = incr_set_uv / vFloat2(2.0f, 3.0f);

	for (int yl=0; yl<ViewMeshSize.y; ++yl) {
		for (int xl=0; xl<ViewMeshSize.x; ++xl) {
			int y = yl + viewport_offset.y;
			int x = xl + viewport_offset.x;
			int instanceId	= ((yl*ViewMeshSize.x) + xl);
			int vertexId	= instanceId * 6;

			if (y<0 || x<0)						{ g_ViewTileID[instanceId] = 31; continue; }
			if (y>=WorldSizeY || x>=WorldSizeX) { g_ViewTileID[instanceId] = 31; continue; }

			g_ViewTileID[instanceId] = terrain[(y * WorldSizeX) + x].tilesetId;
			continue;

			int setId		= terrain[(y * WorldSizeX) + x].tilesetId;
			int setX		= setId % m_setCount.x;
			int setY		= setId / m_setCount.x;

			// Look at surrounding tiles to decide how to match this tile...
			// TODO: Try moving this into Lua?  As a proof-of-concept for rapid iteration?
			//    Or is this not appropriate scope for; scripting yet?  Hmm!

			bool match_above1 = false;
			bool match_below1 = false;
			bool match_left1  = false;
			bool match_right1 = false;

			if (y > 0) {
				int idx = ((y-1) * WorldSizeX) + (x+0);
				match_above1 = (terrain[idx].tilesetId == setId);
			}

			if (y < WorldSizeY-1) {
				int idx = ((y+1) * WorldSizeX) + (x+0);
				match_below1 = (terrain[idx].tilesetId == setId);
			}

			if (x > 0) {
				int idx  = ((y+0) * WorldSizeX) + (x-1);
				match_left1 = (terrain[idx].tilesetId == setId);
			}

			if (x < WorldSizeX-1) {
				int idx = ((y+0) * WorldSizeX) + (x+1);
				match_right1 = (terrain[idx].tilesetId == setId);
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

			g_ViewTileID[instanceId]  = (setY * m_setCount.x) + setX;
			g_ViewTileID[instanceId] += (subTileY * 4) + subTileX;
		}
	}

	// Sort various sprite batches

	dx11_UploadDynamicBufferData(gpu.mesh_worldViewTileID, g_ViewTileID,  sizeof(g_ViewTileID[0]) * ViewInstanceCount);
}

void TileMapLayer::InitScene(const char* script_objname)
{
	if (!script_objname) {
		script_objname = Entity_LookupName(m_gid);
	}

	auto& script = g_scriptEnv;

	//if (auto& worldViewTab = script.glob_open_table(script_objname))
	//{
	//	auto tileSheetFilename	= worldViewTab.get_string("TileSheet");
	//
	//	if (auto getMeshSize = worldViewTab.push_func("getMeshSize")) {
	//		getMeshSize.pusharg("desktop");
	//		getMeshSize.pusharg(1920.0f);
	//		getMeshSize.pusharg(1080.0f);
	//		getMeshSize.execcall(2);		// 2 - num return values
	//		float SizeX = getMeshSize.getresult<float>();
	//		float SizeY = getMeshSize.getresult<float>();
	//
	//		ViewMeshSizeX = int(std::ceilf(SizeX / TileSizeX));
	//		ViewMeshSizeY = int(std::ceilf(SizeY / TileSizeY));
	//
	//		ViewMeshSizeX = std::min(ViewMeshSizeX, WorldSizeX);
	//		ViewMeshSizeY = std::min(ViewMeshSizeY, WorldSizeY);
	//	}
	//}

	// Add +1 to cover overlap area when tile is not "centered" on the screen
	// TODO: determine actual overage to render based on viewcamera angle.
	//ViewMeshSize = int2(ceilf((g_ViewCamera.m_frustrum_in_tiles + 1) * 1.50f));
	ViewMeshSize = int2(ceilf((g_ViewCamera.m_frustrum_in_tiles + 1) * 1.20f));		// for now this is OK

	ViewInstanceCount  = ViewMeshSize.y * ViewMeshSize.x;
	ViewVerticiesCount = ViewInstanceCount * 6;

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

	dx11_CreateStaticMesh(gpu.mesh_tile, g_mesh_UniformQuad,	sizeof(g_mesh_UniformQuad[0]),	bulkof(g_mesh_UniformQuad));
	dx11_CreateDynamicVertexBuffer(gpu.mesh_worldViewTileID, sizeof(g_ViewTileID[0]) * ViewInstanceCount);

	dx11_LoadShaderVS(g_ShaderVS_Tiler, "TileMap.fx", "VS");
	dx11_LoadShaderFS(g_ShaderFS_Tiler, "TileMap.fx", "PS");
}

#include "Mouse.h"

// 8x9 .. TODO: convert this into a texture given a simple legend.
const char* TestDot[] = {
	"---------",
	"----+----",
	"--+++++--",
	"+++++++++",
	"+++++++++",
	"--+++++--",
	"----+----",
	"---------",
};

void TileMapLayer::PopulateUVs(const TerrainMapItem* terrain)
{
	auto disp = int2(gpu.consts.TileAlignedDisp);
	disp -= (ViewMeshSize / 2);
	PopulateUVs(terrain, disp);
}

void TileMapLayer::SetSourceTexture(const TextureAtlas& atlas)
{
	m_setCount = atlas.m_bufferSizeInTiles;
	gpu.consts.SrcTexSizeInTiles	= vInt2(m_setCount);
	gpu.consts.SrcTexTileSizePix	= atlas.m_tileSizePix;
	gpu.consts.SrcTexBorderPix		= {1,1};
	gpu.consts.ViewMeshSize			= ViewMeshSize;

	dx11_CreateTexture2D(gpu.tex_floor, atlas, GPU_ResourceFmt_R8G8B8A8_UNORM);
}

void TileMapLayer::CenterViewOn(const float2& dest)
{
	auto newdisp = floorf(dest);

	if (newdisp != TileAlignedDisp) {
		TileAlignedDisp					= floorf(dest);
		gpu.consts.TileAlignedDisp		= TileAlignedDisp;
	}
}


void TileMapLayer::Tick() {
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
