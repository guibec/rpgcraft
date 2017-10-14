#include "PCH-rpgcraft.h"

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

#include "ajek-script.h"
#include "Entity.h"
#include "Sprites.h"
#include "TileMapLayer.h"
#include "DbgFont.h"
#include "Scene.h"
#include "UniformMeshes.h"

#include <DirectXMath.h>

using namespace DirectX;

DECLARE_MODULE_NAME("main");

GPU_IndexBuffer			g_idx_box2D;
GPU_ShaderVS			g_ShaderVS_Tiler;
GPU_ShaderFS			g_ShaderFS_Tiler;

GPU_ShaderVS			g_ShaderVS_Spriter;
GPU_ShaderFS			g_ShaderFS_Spriter;

bool					g_gpu_ForceWireframe	= false;

GPU_TextureResource2D	tex_chars;

static TickableEntityContainer		g_tickable_entities;
static DrawableEntityContainer		g_drawable_entities;


ViewCamera		g_ViewCamera;
TileMapLayer	g_TileMap;

bool s_CanRenderScene = false;

void SceneLogic()
{
	DbgFont_SceneBegin();

	g_TileMap.Tick();

	for(auto& entitem : g_tickable_entities.ForEachForward())
	{
		// Hmm.. might be better to throw on null entity? or log and ignore?
		// Probably log and ignore bydefault with option to bug ...
		auto* entity = Entity_Lookup(entitem.orderId.Gid()).objectptr;
		bug_on_qa(!entity);
		entitem.Tick(entity);
	}

	// Process messages and modifications which have been submitted to view camera here?
	g_ViewCamera.Tick();
}

void SceneInputPoll()
{
	// TODO : Add some keyboard handler magic here ... !!
	//Host_IsKeyPressed('a');
}

//template< typename T >
//T* CreateEntity()
//{
//	T* entity = placement_new(T);
//	entity->m_gid = Entity_Spawn(entity);
//	return entity;
//}

GPU_ConstantBuffer		g_gpu_constbuf;

void ViewCamera::Reset()
{
	// Note: current default values are just for testing ... no other significant meaning

	// Eye and At should move laterally together so that the eye is always looking straight down
	// at a specific point on the map.

	m_Eye	= float4 { 0.0f, 0.5f, -6.0f, 0.0f }.q;
	m_At	= float4 { 0.0f, 0.5f,  0.0f, 0.0f }.q;
	m_Up	= float4 { 0.0f, 1.0f,  0.0f, 0.0f }.q;
}

// Eye and At should move laterally together so that the eye is always looking straight down
// at a specific point on the map (X/Y equal).
// Eye.Z controls the zoom of the view.
// UP : X is angle.  Y is sign-indicator only (flip axis) --  Z is unused?

void ViewCamera::Tick() {
	m_Consts.View		= XMMatrixLookAtLH(m_Eye, m_At, m_Up);
	m_Consts.Projection = g_Projection;
}


void SceneRender()
{

	if (!s_CanRenderScene) return;


	// Clear the back buffer
	dx11_SetRasterState(GPU_Fill_Solid, GPU_Cull_None, GPU_Scissor_Disable);
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

	GPU_ViewCameraConsts	m_ViewConsts;
	m_ViewConsts.View		= XMMatrixTranspose(g_ViewCamera.m_Consts.View);
	m_ViewConsts.Projection = XMMatrixTranspose(g_ViewCamera.m_Consts.Projection);

	dx11_UpdateConstantBuffer(g_gpu_constbuf, &m_ViewConsts);
	dx11_BindConstantBuffer(g_gpu_constbuf, 0);
	dx11_SetPrimType(GPU_PRIM_TRIANGLELIST);

	g_TileMap.Draw();

	for(const auto& entitem : g_drawable_entities.ForEachAlpha())
	{
		auto* entity = Entity_Lookup(entitem.orderId.Gid()).objectptr;
		bug_on_qa(!entity);
		entitem.Draw(entity);
	}

	//g_pSwapChain->Present(1, DXGI_SWAP_EFFECT_SEQUENTIAL);

	DbgFont_SceneRender();
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

bool Scene_TryLoadInit()
{
	s_CanRenderScene = false;

	// Make sure to clear previous Schene/State information:

	g_tickable_entities.Clear();
	g_drawable_entities.Clear();

	UniformMeshes_InitGlobalResources();

#if 1
	// Fetch Scene configuration from Lua.
	g_scriptEnv.LoadModule("scripts/GameInit.lua");

	if (g_scriptEnv.HasError()) {
		return false;
	}
#endif

	if (1) {
		xBitmapData  pngtex;
		png_LoadFromFile(pngtex, ".\\sheets\\characters\\don_collection_27_20120604_1722740153.png");
		dx11_CreateTexture2D(tex_chars, pngtex.buffer.GetPtr(), pngtex.width, pngtex.height, GPU_ResourceFmt_R8G8B8A8_UNORM);
	}

	dx11_LoadShaderVS(g_ShaderVS_Spriter, "Sprite.fx", "VS");
	dx11_LoadShaderFS(g_ShaderFS_Spriter, "Sprite.fx", "PS");

	PlaceEntity(g_ViewCamera);
	PlaceEntity(g_TileMap);

	auto* player	= NewEntity(PlayerSprite);

	g_ViewCamera.Reset();
	g_TileMap.SceneInit("WorldView");

	g_tickable_entities.Add(10, player->m_gid,		[](      void* entity) { ((PlayerSprite*)entity)->Tick(); } );
	g_drawable_entities.Add(10, player->m_gid,		[](const void* entity) { ((PlayerSprite*)entity)->Draw(); } );

	dx11_CreateConstantBuffer(g_gpu_constbuf,		sizeof(GPU_ViewCameraConsts));

	s_CanRenderScene = 1;
	return true;
}
