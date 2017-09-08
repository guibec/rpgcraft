
#include "x-types.h"
#include "x-stl.h"
#include "x-assertion.h"
#include "x-string.h"
#include "x-thread.h"

#include "x-host-ifc.h"
#include "x-gpu-ifc.h"
#include "x-gpu-colors.h"
#include "x-png-decode.h"
#include "v-float.h"

#include "Entity.h"
#include "UniformMeshes.h"

#include <DirectXMath.h>

using namespace DirectX;

DECLARE_MODULE_NAME("main");

GPU_VertexBuffer		g_mesh_box2D;
GPU_VertexBuffer		g_mesh_tile;
GPU_VertexBuffer		g_mesh_worldViewTileID;

GPU_IndexBuffer			g_idx_box2D;
GPU_ShaderVS			g_ShaderVS_Tiler;
GPU_ShaderFS			g_ShaderFS_Tiler;

GPU_ShaderVS			g_ShaderVS_Spriter;
GPU_ShaderFS			g_ShaderFS_Spriter;

bool					g_gpu_ForceWireframe	= false;

GPU_TextureResource2D	tex_floor;
GPU_TextureResource2D	tex_chars;
GPU_TextureResource2D	tex_terrain;

static const int TileSizeX = 8;
static const int TileSizeY = 8;

// TODO: Make this dynamic ...

//static const int ViewMeshSizeX		= 128;
//static const int ViewMeshSizeY		= 96;
static int ViewMeshSizeX			= 24;
static int ViewMeshSizeY			= 24;
static int worldViewInstanceCount	= 0;
static int worldViewVerticiesCount	= 0;

// ------------------------------------------------------------------------------------------------
// struct TileMapVertex
// ------------------------------------------------------------------------------------------------
//  * Used to define the static (generally unchanging) visible tile map.
//  * The UVs are hard-coded to always go from 0.0f to 1.0f across each quad.
//
// Actual UVs are calculated by the pixel shader -- it reads from tex_tile_ids to determine
// the base offset of the tile, and then adds the VS-interpolated value range 0.0f - 1.0f to that
// to smaple the texture.
//
// Note: Requires multi-resource support by the GPU.  No big deal for deaktops and consoles.  May
//       not be widely available on mobile devices; or maybe it will be by the time we're interested
//       in considering shipping the title for mobile.  So just going to assume multi-texture support
//       for now... --jstine

struct TileMapVertex {
	vFloat3		xyz;
	vFloat2		uv;
};

static const int WorldSizeX		= 1024;
static const int WorldSizeY		= 1024;


// xyz should probably fixed.  Only the camera and the UVs need to change.
// UV, Lighting should be independenty stored in the future, to allow them to be updated at different update intervals.
//    * 30fps for UV, 10fps for lighting, etc.

struct TileMapVertexLit {
	vFloat3		xyz;
	vFloat2		uv;
	//vFloat4     rgba;		// current light intensity
};

// Probably need some sort of classification system here.
// Some terrains may change over time, such as grow moss after being crafted.
//  - Maybe better handled as a generic "age" engine feature?
//  - But there could be different types of mosses, or stalagmites, or other environment changes.

struct TerrainMapItem {
	int		tilesetId;		// specific tile from the set is determined according to surrounding tiles at render time.
};

TerrainMapItem*		g_WorldMap;
u32*				g_ViewTileID;	// temporarily global - will localize later.

// Probably in tile coordinates with fractional being partial-tile position
float g_playerX;
float g_playerY;


static const int TerrainTileW = 256;
static const int TerrainTileH = 256;

static TickableEntityContainer		g_tickable_entities;
static DrawableEntityContainer		g_drawable_entities;

int g_setCountX = 0;
int g_setCountY = 0;

void TileMapView_PopulateUVs()
{
	// Populate view mesh according to world map information:

	vFloat2 incr_set_uv = vFloat2(1.0f / g_setCountX, 1.0f / g_setCountY);
	vFloat2 t16uv = incr_set_uv / vFloat2(2.0f, 3.0f);

	for (int y=0; y<ViewMeshSizeY; ++y) {
		for (int x=0; x<ViewMeshSizeX; ++x) {
			int instanceId	= ((y*ViewMeshSizeX) + x);
			int vertexId	= instanceId * 6;

			int setId		= g_WorldMap[(y * WorldSizeX) + x].tilesetId;
			int setX		= setId % g_setCountX;
			int setY		= setId / g_setCountX;

			// Look at surrounding tiles to decide how to match this tile...
			// TODO: Try moving this into Lua?  As a proof-of-concept for rapid iteration?
			//    Or is this not appropriate scope for scripting yet?  Hmm!

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

			vFloat2 uv;
			uv  = vFloat2(setX, setY)  * incr_set_uv;
			uv += vFloat2(subTileX, subTileY) * t16uv;
		}
	}

	// Sort various sprite batches
}


bool s_CanRenderScene = false;

void SceneBegin()
{
	TileMapView_PopulateUVs();
}

void SceneInputLogic()
{
	// TODO : Add some keyboard handler magic here ... !!
	//Host_IsKeyPressed('a');
}

static u32 s_entity_spawn_id = 1;

#include "x-atomic.h"

u32 getNewEntitySpawnId()
{
	int result = AtomicInc((s32&)s_entity_spawn_id);
	if (result == 0) {
		// wrapped around the world.  impressive.
		log_host("super-secret impossible easter egg found!");
		// but zero is an invalid spawn ID, so discard it:
		result = AtomicInc((s32&)s_entity_spawn_id);
	}
	return result;
}

template< typename T >
T* CreateEntity()
{
	T* entity = placement_new(T);
	return entity;
}

class BasicEntitySpawnId : public virtual ISpawnId
{
private:
	NONCOPYABLE_OBJECT(BasicEntitySpawnId);

public:
	u32	 m_spawnId = 0;

protected:
	BasicEntitySpawnId() {
		auto newSpawnId = getNewEntitySpawnId();
		m_spawnId		= newSpawnId;
	}

public:
	virtual u32 GetSpawnId() const {
		return m_spawnId;
	}
};

struct GPU_ViewCameraConsts
{
	XMMATRIX View;
	XMMATRIX Projection;
};

struct GPU_TileMapConstants
{
	vFloat2 TileAlignedDisp;		// TODO: move calculation of this to shader.
	vInt2	SrcTexSizeInTiles;
	vFloat2	SrcTexTileSizeUV;
	u32		TileMapSizeX;
	u32		TileMapSizeY;
};

GPU_ConstantBuffer		g_gpu_constbuf;
GPU_ConstantBuffer		g_cnstbuf_TileMap;

extern XMMATRIX         g_Projection;

class ViewCamera :
	public virtual BasicEntitySpawnId,
	public virtual ITickableEntity
{
public:
	u128					m_Eye;
	u128					m_At;
	u128					m_Up;			// X is angle.  Y is just +/- (orientation)? Z is unused?
	GPU_ViewCameraConsts	m_Consts;

	ViewCamera() : BasicEntitySpawnId() {
	}

	// Eye and At should move laterally together so that the eye is always looking straight down
	// at a specific point on the map (X/Y equal).
	// Eye.Z controls the zoom of the view.
	// UP : X is angle.  Y is sign-indicator only (flip axis) --  Z is unused?

	virtual void Tick() {
		m_Eye	= XMVectorSet( 0.0f, 0.5f, -6.0f, 0.0f );
		m_At	= XMVectorSet( 0.0f, 0.5f,  0.0f, 0.0f );
		m_Up	= XMVectorSet( 0.0f, 1.0f,  0.0f, 0.0f );

		m_Consts.View		= XMMatrixLookAtLH(m_Eye, m_At, m_Up);
		m_Consts.Projection = g_Projection;
	}
};

class TileMapLayer :
	public virtual BasicEntitySpawnId,
	public virtual ITickableEntity
{
public:
	TileMapLayer() : BasicEntitySpawnId() {
	}

	// Eye and At should move laterally together so that the eye is always looking straight down
	// at a specific point on the map.

	virtual void Tick() {
	}

	virtual void Draw() const;
};

static ViewCamera		g_ViewCamera;
static TileMapLayer*	g_TileMap;

void TileMapLayer::Draw() const
{
	GPU_TileMapConstants	m_TileMapConsts;

	// determine tile map draw position according to camera position.

	m_TileMapConsts.TileAlignedDisp		= vFloat2(floorf(g_ViewCamera.m_Eye.xf), floorf(g_ViewCamera.m_Eye.yf));
	m_TileMapConsts.SrcTexSizeInTiles	= vInt2(g_setCountX, g_setCountY);
	m_TileMapConsts.SrcTexTileSizeUV	= vFloat2(1.0f / g_setCountX, 1.0f / g_setCountY) / vFloat2(2.0f, 3.0f);
	m_TileMapConsts.TileMapSizeX		= ViewMeshSizeX;
	m_TileMapConsts.TileMapSizeY		= ViewMeshSizeY;

	dx11_BindShaderVS(g_ShaderVS_Tiler);
	dx11_BindShaderFS(g_ShaderFS_Tiler);
	dx11_SetInputLayout(VertexBufferLayout_TileMap);

//	dx11_SetPrimType(GPU_PRIM_TRIANGLELIST);
	dx11_BindShaderResource(tex_floor, 0);

	dx11_SetVertexBuffer(g_mesh_tile,				0, sizeof(g_mesh_UniformQuad[0]), 0);
	dx11_SetVertexBuffer(g_mesh_worldViewTileID,	1, sizeof(g_ViewTileID[0]), 0);
	//dx11_SetVertexBuffer(g_mesh_worldViewColor, 2, sizeof(g_ViewUV[0]), 0);

	dx11_UpdateConstantBuffer(g_cnstbuf_TileMap, &m_TileMapConsts);
	dx11_BindConstantBuffer(g_cnstbuf_TileMap, 1);
	dx11_SetIndexBuffer(g_idx_box2D, 16, 0);
	dx11_DrawIndexedInstanced(6, worldViewInstanceCount, 0, 0, 0);

}

class PlayerSprite :
	public virtual BasicEntitySpawnId,
	public virtual IDrawableEntity,
	public virtual ITickableEntity
{
private:
	NONCOPYABLE_OBJECT(PlayerSprite);

public:

public:
	PlayerSprite() : BasicEntitySpawnId() {
	}

public:
	virtual void Tick()
	{
	}

	virtual void Draw() const
	{
		dx11_BindShaderVS		(g_ShaderVS_Spriter);
		dx11_BindShaderFS		(g_ShaderFS_Spriter);
		dx11_SetInputLayout		(VertexBufferLayout_Tex1);
		dx11_BindShaderResource	(tex_chars, 0);
		dx11_SetVertexBuffer	(g_mesh_box2D, 0, sizeof(TileMapVertex), 0);
		dx11_SetIndexBuffer		(g_idx_box2D, 16, 0);

		dx11_DrawIndexed(6, 0,  0);
	}
};

extern void DbgFont_Render();

void SceneRender()
{
	if (!s_CanRenderScene) return;

	// Clear the back buffer
	dx11_ClearRenderTarget(g_gpu_BackBuffer, GPU_Colors::MidnightBlue);

	//
	// Update variables
	//
	//ConstantBuffer cb;
	//cb.mWorld = XMMatrixTranspose(g_World);
	//cb.mView = XMMatrixTranspose(g_View);
	//cb.mProjection = XMMatrixTranspose(g_Projection);
	//g_pImmediateContext->UpdateSubresource(g_pConstantBuffer, 0, nullptr, &cb, 0, 0);

	// ------------------------------------------------------------------------------------------
	// Renders Scene Geometry
	//

	dx11_SetRasterState(GPU_Fill_Solid, GPU_Cull_None, GPU_Scissor_Disable);

	for(auto& entitem : g_tickable_entities.ForEachForward())
	{
		auto* const& entity = entitem.entity;
		entity->Tick();
	}

	GPU_ViewCameraConsts	m_ViewConsts;
	m_ViewConsts.View		= XMMatrixTranspose(g_ViewCamera.m_Consts.View);
	m_ViewConsts.Projection = XMMatrixTranspose(g_ViewCamera.m_Consts.Projection);

	dx11_UpdateConstantBuffer(g_gpu_constbuf, &m_ViewConsts);
	dx11_BindConstantBuffer(g_gpu_constbuf, 0);
	dx11_SetPrimType(GPU_PRIM_TRIANGLELIST);

	g_TileMap->Draw();

	for(const auto& entitem : g_drawable_entities.ForEachAlpha())
	{
		const auto* const& entity = entitem.entity;
		entity->Draw();
	}

	//g_pSwapChain->Present(1, DXGI_SWAP_EFFECT_SEQUENTIAL);

	DbgFont_Render();

	dx11_BackbufferSwap();
}

// Size notes:
//   * Tiles are 16x16
//   * Full coverage of 1920x1080 is approximately 120x68 titles.

//   * 1024x1024 tile world comes to 96mb -- which is close to 12-screens worth of world to explore
//      * OK for single player, but multiplayer should be able to go larger.
//   * If breaking mesh into smaller chunks, then chunks should be ~64x48 in size
//
// Lighting notes:
//   * Ambient lighting is a shader constant (easy!)
//   * Lights should support wall/object occlusion
//      * In simple terms, lights passing through dense material are occluded by some percentage
//      * Use standard circular light texture, broken into 16x16 grid of tiles  (size is example only)
//      * At points where the light is being drawn on top of "solid" object, *fold* the vertices inward
//        to distort the map accordingly.
//      * Vertices on the far side of object continue to radiate normally, similar to how water distorts
//        a ray of light only for the time it's passing through the material.
//   * Terraria uses a "flare" system where a light casts four distinct lines of luminence: up, down, left, right.
//      * Luminence then radiates in straight perepndicular lines along each flare (2nd pass)
//      * Each pass uses procedural stepping to propagate the flare/light (solid materials decay propagation quicker)
//      * There may also be some amount of ambient light spillover (liquid-style) in some cases... can figure that out later.
//   * Terraria also uses some pre-pass to truncate flare lengths to avoid "unnecessarily" overlapping lights.
//      * Flares of both lights stop at the midpoint between the two lights.
//      * Only affects lights in wide-open areas (likely calculated as a "max extent" of the flare through transparent material)
//      * This actually causes "shadows" or "holes" to form in the light-coverage when two torches are placed eithin 30-ish
//        tiles but offset on the Y-axis by a few titles.
//   * Terraria updates lighting at ~10fps, movement of lights is noticably behind player.
//

#include "ajek-script.h"

bool Scene_TryLoadInit(AjekScriptEnv& script)
{
	s_CanRenderScene = false;

	// default test values in case script loading is bypassed.
	ViewMeshSizeX = 24;
	ViewMeshSizeY = 24;
	worldViewInstanceCount	= ViewMeshSizeY * ViewMeshSizeX;
	worldViewVerticiesCount = worldViewInstanceCount * 6;

#if 1
	// Fetch Scene configuration from Lua.
	script.LoadModule("scripts/GameInit.lua");

	if (script.HasError()) {
		return false;
	}

	//if (auto& worldTab = script.glob_open_table("World"))
	//{
	//	WorldSizeX = worldTab.get<u32>("size");
	//	WorldSizeX = worldTab.get<u32>("size");
	//}

	if (auto& worldViewTab = script.glob_open_table("WorldView"))
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
			worldViewInstanceCount  = ViewMeshSizeY * ViewMeshSizeX;
			worldViewVerticiesCount = worldViewInstanceCount * 6;
		}
	}
#endif

	if (1) {
		xBitmapData  pngtex;
		png_LoadFromFile(pngtex, "..\\rpg_maker_vx__modernrtp_tilea2_by_painhurt-d3f7rwg.png");
		dx11_CreateTexture2D(tex_floor, pngtex.buffer.GetPtr(), pngtex.width, pngtex.height, GPU_ResourceFmt_R8G8B8A8_UNORM);

		// Assume pngtex is rpgmaker layout for now.

		g_setCountX = pngtex.width	/ 64;
		g_setCountY = pngtex.height	/ (64 + 32);
	}

	if (1) {
		xBitmapData  pngtex;
		png_LoadFromFile(pngtex, "..\\sheets\\characters\\don_collection_27_20120604_1722740153.png");
		dx11_CreateTexture2D(tex_chars, pngtex.buffer.GetPtr(), pngtex.width, pngtex.height, GPU_ResourceFmt_R8G8B8A8_UNORM);
	}

	xFree(g_WorldMap);		g_WorldMap		= nullptr;
	xFree(g_ViewTileID);	g_ViewTileID	= nullptr;

	g_WorldMap	= (TerrainMapItem*)	xMalloc(WorldSizeX    * WorldSizeY    * sizeof(TerrainMapItem));
	g_ViewTileID= (u32*)			xMalloc(worldViewInstanceCount	* sizeof(u32));

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
		g_WorldMap[((     0	     ) * WorldSizeX) + x].tilesetId = 20;
		g_WorldMap[((     1	     ) * WorldSizeX) + x].tilesetId = 20;
		g_WorldMap[((WorldSizeY-2) * WorldSizeX) + x].tilesetId = 20;
		g_WorldMap[((WorldSizeY-1) * WorldSizeX) + x].tilesetId = 20;
	}

	for (int y=0; y<WorldSizeY; ++y) {
		g_WorldMap[(y * WorldSizeX) + 0				].tilesetId = 20;
		g_WorldMap[(y * WorldSizeX) + 1				].tilesetId = 20;
		g_WorldMap[(y * WorldSizeX) + (WorldSizeX-2)].tilesetId = 20;
		g_WorldMap[(y * WorldSizeX) + (WorldSizeX-1)].tilesetId = 20;
	}

	TileMapView_PopulateUVs();

	dx11_CreateStaticMesh(g_mesh_tile,				g_mesh_UniformQuad,	sizeof(g_mesh_UniformQuad[0]),	bulkof(g_mesh_UniformQuad));
	dx11_CreateStaticMesh(g_mesh_worldViewTileID,	g_ViewTileID,		sizeof(g_ViewTileID[0]),		worldViewInstanceCount);

	dx11_LoadShaderVS(g_ShaderVS_Tiler, "TileMap.fx", "VS");
	dx11_LoadShaderFS(g_ShaderFS_Tiler, "TileMap.fx", "PS");

	dx11_LoadShaderVS(g_ShaderVS_Spriter, "Sprite.fx", "VS");
	dx11_LoadShaderFS(g_ShaderFS_Spriter, "Sprite.fx", "PS");

	// ---------------------------------------------------------------------------------------------

	// TODO - Base the sprite mesh size on the tile size of the world map .. ?
	// Such that all sprites are described in tile units (floats/fractional ok).
	// and tile units have a defined pixel size, so we can convert from pixels->tiles for
	// precision tile/sprite placement.
	// woo!!!


	TileMapVertex vertices[] =
	{
		// UV is in pixels (as it should be).
		// XYZ is in tiles .. where Z is expected to be 1.0f

		{ vFloat3( -0.5f,  0.5f, 0.0f ), vFloat2( 0.0f, 32.0f) },
		{ vFloat3( -0.5f, -0.5f, 0.0f ), vFloat2( 0.0f, 64.0f) },
		{ vFloat3(  0.5f, -0.5f, 0.0f ), vFloat2(24.0f, 64.0f) },
		{ vFloat3(  0.5f,  0.5f, 0.0f ), vFloat2(24.0f, 32.0f) }
	};

	dx11_CreateStaticMesh(g_mesh_box2D, vertices, sizeof(vertices[0]), bulkof(vertices));
	dx11_CreateIndexBuffer(g_idx_box2D, g_ind_UniformQuad, sizeof(g_ind_UniformQuad));
	// ---------------------------------------------------------------------------------------------

	g_TileMap = CreateEntity<TileMapLayer>();

	auto* player = CreateEntity<PlayerSprite>();
	g_tickable_entities.Add(&g_ViewCamera, 1);
	g_tickable_entities.Add(player, 10);
	g_drawable_entities.Add(player, 10);

	dx11_CreateConstantBuffer(g_gpu_constbuf,		sizeof(GPU_ViewCameraConsts));
	dx11_CreateConstantBuffer(g_cnstbuf_TileMap,	sizeof(GPU_TileMapConstants));

	s_CanRenderScene = 1;
	return true;
}
