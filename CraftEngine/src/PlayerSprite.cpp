
#include "PCH-rpgcraft.h"
#include "Sprites.h"
#include "DbgFont.h"

#include "x-pad.h"
#include "TileMapLayer.h"
#include "Scene.h"
#include "Mouse.h"

DECLARE_MODULE_NAME("player");

GPU_ConstantBuffer gpu_constbuf;

PlayerSprite::PlayerSprite() {
	xMemZero(gpu_layout_sprite);
	gpu_layout_sprite.AddVertexSlot( {
		{ "POSITION", GPU_ResourceFmt_R32G32B32_FLOAT	},
		{ "TEXCOORD", GPU_ResourceFmt_R32G32_FLOAT		}
	});

	// ---------------------------------------------------------------------------------------------
	TileMapVertex vertices[] =
	{
		// UV is in pixels (as it should be).
		// XY is in tiles .. where Z is expected to be 1.0f

		//{ vFloat3( -0.5f, -0.5f, 0.0f ), vFloat2( 0.0f, 32.0f) },
		//{ vFloat3( -0.5f,  0.5f, 0.0f ), vFloat2( 0.0f, 64.0f) },
		//{ vFloat3(  0.5f,  0.5f, 0.0f ), vFloat2(24.0f, 64.0f) },
		//{ vFloat3(  0.5f, -0.5f, 0.0f ), vFloat2(24.0f, 32.0f) }

		{ vFloat3(  0.0f,  0.0f, 1.0f ), vFloat2( 0.0f, 32.0f) },
		{ vFloat3(  0.0f,  1.0f, 1.0f ), vFloat2( 0.0f, 64.0f) },
		{ vFloat3(  1.0f,  1.0f, 1.0f ), vFloat2(24.0f, 64.0f) },
		{ vFloat3(  1.0f,  0.0f, 1.0f ), vFloat2(24.0f, 32.0f) }
	};

	dx11_CreateStaticMesh(gpu_mesh_box2D, vertices, sizeof(vertices[0]), bulkof(vertices));
	// ---------------------------------------------------------------------------------------------

	dx11_CreateConstantBuffer(gpu_constbuf, sizeof(float2) + sizeof(int2));

	m_position = { 10, 10 };
}

extern float4 get3dPoint(const int2& viewPos, const int2& viewSize, const XMMATRIX& viewMatrix, const XMMATRIX& projectionMatrix);

static bool s_isAbsolute = false;

void PlayerSprite::Tick(int order)
{
	PadState state;
	KPad_GetState(state);

	//g_ViewCamera.SetEyeAt( {
	//	g_ViewCamera.m_Eye.x + (state.axis.RStick_X * 0.05f),
	//	g_ViewCamera.m_Eye.y + (state.axis.RStick_Y * 0.05f),
	//} );

	g_ViewCamera.m_At += { state.axis.RStick_X * 0.05f, state.axis.RStick_Y * 0.05f };

	const float playerSpeed = 1.f; // 1 tile per second

	float2 direction;
	direction.x = 0;
	direction.y = 0;

	if (Scene_IsKeyPressed(VirtKey::ArrowUp))
	{
		direction.y = -playerSpeed;
	}
	else if (Scene_IsKeyPressed(VirtKey::ArrowDown))
	{
		direction.y = playerSpeed;
	}
	if (Scene_IsKeyPressed(VirtKey::ArrowLeft))
	{
		direction.x = -playerSpeed;
	}
	else if (Scene_IsKeyPressed(VirtKey::ArrowRight))
	{
		direction.x = playerSpeed;
	}

	if (state.axis.LStick_X < 0)
		direction.x = -playerSpeed;
	else if (state.axis.LStick_X > 0)
		direction.x = playerSpeed;
	if (state.axis.LStick_Y < 0)
		direction.y = -playerSpeed;
	else if (state.axis.LStick_Y > 0)
		direction.y = playerSpeed;

	float dt = 1 / 60.f; // need a Timer/FPS method

	m_position += direction * dt;


	ImGui::Checkbox("Absolute Position Test Mode", &s_isAbsolute);

	if (s_isAbsolute){
		if (g_mouse.isTrackable()) {
			// absolute float under cursor (for diagnostic!)
			auto mouse = g_mouse.clientToNormal();
			auto tilepos = mouse.normal * g_ViewCamera.m_frustrum_in_tiles.y / 2.f;
			tilepos += (g_TileMap.ViewMeshSize * 0.5f);
			tilepos += float2 { g_ViewCamera.m_Eye.x, g_ViewCamera.m_Eye.y };

			ImGui::Text("MousePos = %5.2f %5.2f", tilepos.x, tilepos.y);
			ImGui::Text("Frustrum = %5.2f %5.2f", g_ViewCamera.m_frustrum_in_tiles.x, g_ViewCamera.m_frustrum_in_tiles.y);
			m_position = tilepos;
		}
	}

	g_drawlist_main.Add(this, 1);
}


void PlayerSprite::Draw(int order) const
{
	// TODO: most of this is generic for all sprites and should be default for all items
	// in the sprites render list.  Moreover, the API should be such that any locally-modified GPU
	// states should be restored after the sprite draw is finished.

	dx11_BindShaderVS		(g_ShaderVS_Spriter);
	dx11_BindShaderFS		(g_ShaderFS_Spriter);
	dx11_SetInputLayout		(gpu_layout_sprite);
	dx11_BindShaderResource	(tex_chars, 0);
	dx11_SetVertexBuffer	(gpu_mesh_box2D, 0, sizeof(TileMapVertex), 0);
	dx11_SetIndexBuffer		(g_idx_box2D, 16, 0);

	struct {
		float2	relpos;
		int2	TileMapSize;
	} consts;

	consts.relpos			= m_position;
	consts.TileMapSize		= g_TileMap.ViewMeshSize;

	dx11_UpdateConstantBuffer	(gpu_constbuf, &consts);
	dx11_BindConstantBuffer		(gpu_constbuf, 1);

	dx11_DrawIndexed(6, 0,  0);
}

void UniformMeshes_InitGlobalResources() {
	dx11_CreateIndexBuffer(g_idx_box2D, g_ind_UniformQuad, sizeof(g_ind_UniformQuad));
}