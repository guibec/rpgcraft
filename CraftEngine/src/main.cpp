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

#include "imgui_impl_dx11.h"
#include "imgui-console.h"

#include "Mouse.h"

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
Mouse			g_mouse;

bool s_CanRenderScene = false;

float2 get2dPoint(const float4& point3D, const int2& viewSize, const XMMATRIX& viewMatrix, const XMMATRIX& projectionMatrix)
{
	auto viewProjectionMatrix = projectionMatrix * viewMatrix;

	//transform world to clipping coordinates
	auto _xform = XMVector3Transform(point3D, viewProjectionMatrix);
	float4& xform = (float4&)_xform;

	float4 xf2 = xform / xform.w;

	// This assumes viewOffset is center of the screen, directx style.
	float winX = int (roundf((( xform.x + 1 ) / 2.0) * viewSize.x ));
	float winY = int (roundf((( 1 - xform.y ) / 2.0) * viewSize.y ));

	// More generic OGL friendly version but I need to determine exactly what viewOffset should be, and I'm lazy --jstine
	//float winX = ((xf2.x + 1.0) / 2.0) * viewSize.x; // + viewOffset
	//float winY = ((xf2.y + 1.0) / 2.0) * viewSize.y; // + viewOffset

	return { winX, winY };
}

float4 get3dPoint(const int2& viewPos, const int2& viewSize, const XMMATRIX& viewMatrix, const XMMATRIX& projectionMatrix)
{
	// Currently a bit of a useless function.  It merely returns a ray with no specific Z value.
	// Ray casting needs to be performed from the camera through the world to determine what objects
	// of relevance are intersected.  I would prefer using depth stencil instead and letting the GPU
	// tell us exactly what's under the mouse.  This is left in for reference --jstine

	auto x =   2.0f * viewPos.x / viewSize.x - 1.f;
	auto y = - 2.0f * viewPos.y / viewSize.y + 1.f;

	// should this be inverse of both or just inverse of projection matrix?
	XMMATRIX viewProjectionInverse = XMMatrixInverse(nullptr, projectionMatrix * viewMatrix);
	float4 point3D = { x, y, 1.f };
	auto result = XMVector3Transform(point3D, viewProjectionInverse);

	// TODO: subtract camera position?

	return (float4&)result;
}

bool Scene_IsKeyPressed(VirtKey_t vk_code)
{
	if (!g_mouse.hasFocus())		{ return false; }
	return Host_IsKeyPressedGlobally(vk_code);
}

void SceneLogic()
{
	g_mouse.update();

	{
		static float f = 0.0f;
		//ImGui::Text("Hello, world!");
		//ImGui::SliderFloat("float", &f, 0.0f, 1.0f);
		//ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
	}

	g_console.DrawFrame();
	g_TileMap.Tick();

	for(auto& entitem : g_tickable_entities.ForEachForward())
	{
		// Hmm.. might be better to throw on null entity? or log and ignore?
		// Probably log and ignore bydefault with option to bug ...
		auto* entity = Entity_Lookup(entitem.orderGidPair.Gid()).objectptr;
		bug_on_qa(!entity);
		entitem.Tick(entity, entitem.orderGidPair.Order());
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

	m_Eye	= { 0.0f, 0.5f, -1.0f, 0.0f };
	m_At	= { 0.0f, 0.5f,  1.0f, 0.0f };
	m_Up	= { 0.0f, 1.0f,  0.0f, 0.0f };

	m_aspect				= g_backbuffer_aspect_ratio;
	m_frustrum_in_tiles		= {14 * m_aspect, 14};
}

// Eye and At should move laterally together so that the eye is always looking straight down
// at a specific point on the map (X/Y equal).
// Eye.Z controls the zoom of the view.
// UP : X is angle.  Y is sign-indicator only (flip axis) --  Z is unused?

void ViewCamera::Tick() {
	bool usePerspective = 0;

	m_Eye.z				= usePerspective ? -4.0f : -4.0f;
	m_Consts.View		= XMMatrixLookAtLH(m_Eye, m_At, m_Up);
	//m_Consts.Projection = XMMatrixPerspectiveFovLH( XM_PIDIV2, m_aspect, 0.01f, 1000.0f );

	if (usePerspective) {
		// the frustrum is relative to the minimum Z.
		//  * Min Z influences both perspective and clipping
		//  * Max Z influences clipping behavior only, has no effect on perspective
		//  * Frustrum scaled according to MinZ
		float minZ = 0.010f;
		float frustrumscale = fabsf(m_At.z - m_Eye.z);
		m_Consts.Projection = XMMatrixPerspectiveLH (m_frustrum_in_tiles.x*minZ/frustrumscale, m_frustrum_in_tiles.y*minZ/frustrumscale, minZ, 1000.0f);
	}
	else {
		m_Consts.Projection = XMMatrixOrthographicLH(m_frustrum_in_tiles.x, m_frustrum_in_tiles.y, 0.01f, 1000.0f);
	}
}

void ViewCamera::SetEyeAt(const float2& xy)
{
	m_Eye.xy	= xy;
	m_At.xy		= xy;
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
	dx11_BindConstantBuffer	 (g_gpu_constbuf, 0);
	dx11_SetPrimType(GPU_PRIM_TRIANGLELIST);

	g_TileMap.Draw();

	for(const auto& entitem : g_drawable_entities.ForEachAlpha())
	{
		auto* entity = Entity_Lookup(entitem.orderGidPair.Gid()).objectptr;
		bug_on_qa(!entity);
		entitem.Draw(entity, entitem.orderGidPair.Order());
	}
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

	// Fetch Scene configuration from Lua.
	g_scriptEnv.LoadModule("scripts/GameInit.lua");

	if (g_scriptEnv.HasError()) {
		return false;
	}

	if (1) {
		xBitmapData  pngtex;
		png_LoadFromFile(pngtex, ".\\sheets\\characters\\don_collection_27_20120604_1722740153.png");
		dx11_CreateTexture2D(tex_chars, pngtex.buffer.GetPtr(), pngtex.width, pngtex.height, GPU_ResourceFmt_R8G8B8A8_UNORM);
	}

	dx11_LoadShaderVS(g_ShaderVS_Spriter, "Sprite.fx", "VS");
	dx11_LoadShaderFS(g_ShaderFS_Spriter, "Sprite.fx", "PS");

	PlaceEntity(g_ViewCamera);
	PlaceEntity(g_TileMap);

	g_ViewCamera.Reset();
	g_TileMap.SceneInit("WorldView");

	auto* player	= NewEntity(PlayerSprite);

	g_tickable_entities.Add(player, 10);
	g_drawable_entities.Add(player, 10);

	dx11_CreateConstantBuffer(g_gpu_constbuf,		sizeof(GPU_ViewCameraConsts));

	s_CanRenderScene = 1;
	return true;
}


void Host_ImGui_Init()
{
	ImGuiIO& io = ImGui::GetIO();
	io.KeyMap[ImGuiKey_Tab]			= VirtKey::Tab;                       // Keyboard mapping. ImGui will use those indices to peek into the io.KeyDown[] array that we will update during the application lifetime.
	io.KeyMap[ImGuiKey_LeftArrow]	= VirtKey::ArrowLeft;
	io.KeyMap[ImGuiKey_RightArrow]	= VirtKey::ArrowRight;
	io.KeyMap[ImGuiKey_UpArrow]		= VirtKey::ArrowUp;
	io.KeyMap[ImGuiKey_DownArrow]	= VirtKey::ArrowDown;
	io.KeyMap[ImGuiKey_PageUp]		= VirtKey::PageUp;
	io.KeyMap[ImGuiKey_PageDown]	= VirtKey::PageDown;
	io.KeyMap[ImGuiKey_Home]		= VirtKey::Home;
	io.KeyMap[ImGuiKey_End]			= VirtKey::End;
	io.KeyMap[ImGuiKey_Delete]		= VirtKey::Delete;
	io.KeyMap[ImGuiKey_Backspace]	= VirtKey::Backspace;
	io.KeyMap[ImGuiKey_Enter]		= VirtKey::Enter;
	io.KeyMap[ImGuiKey_Escape]		= VirtKey::Escape;
	io.KeyMap[ImGuiKey_A]			= 'A';
	io.KeyMap[ImGuiKey_C]			= 'C';
	io.KeyMap[ImGuiKey_V]			= 'V';
	io.KeyMap[ImGuiKey_X]			= 'X';
	io.KeyMap[ImGuiKey_Y]			= 'Y';
	io.KeyMap[ImGuiKey_Z]			= 'Z';
}

void Host_ImGui_NewFrame()
{
	_hostImpl_ImGui_NewFrame();
}
