
#include "PCH-rpgcraft.h"
#include "Sprites.h"
#include "DbgFont.h"

#include "x-pad.h"
#include "x-BitmapData.h"
#include "x-png-decode.h"
#include "imgtools.h"

#include "TileMapLayer.h"
#include "Scene.h"
#include "Mouse.h"

DECLARE_MODULE_NAME("player");

GPU_ConstantBuffer		gpu_constbuf;
GPU_TextureResource2D	tex_camel[3];
GPU_VertexBuffer		gpu_mesh_box2D;

void PlayerSprite::LoadStaticAssets()
{
	xBitmapData		pngsrc;
	xBitmapData		texbmp;

	// Because we're using an RPGMaker style sprite sheet sample:
	//   Cut sprites from the source image and paste them into a well-formed GPU texture.

	png_LoadFromFile(pngsrc, ".\\sheets\\characters\\don_collection_27_20120604_1722740153.png");
	//imgtool::ConvertOpaqueColorToAlpha(pngsrc, rgba32(0x007575));		// git-checked copy is currently pre-converted via imagemagick...

	xBitmapData		curtex;
	int2			cutpos  = { 0, 32 };
	int2			cutsize = { 24, 32 };
	for (int i=0; i<3; ++i, cutpos.x += cutsize.x) {
		imgtool::CutTex(curtex, pngsrc, cutpos, cutpos + cutsize);
		dx11_CreateTexture2D(tex_camel[2-i], curtex.buffer.GetPtr(), curtex.size, GPU_ResourceFmt_R8G8B8A8_UNORM);
	}

	// ---------------------------------------------------------------------------------------------
	TileMapVertex vertices[] =
	{
		// UV is in pixels (as it should be).
		// XY is in tiles .. where Z is expected to be 1.0f

		//{ vFloat3( -0.5f, -0.5f, 0.0f ), vFloat2( 0.0f, 32.0f) },
		//{ vFloat3( -0.5f,  0.5f, 0.0f ), vFloat2( 0.0f, 64.0f) },
		//{ vFloat3(  0.5f,  0.5f, 0.0f ), vFloat2(24.0f, 64.0f) },
		//{ vFloat3(  0.5f, -0.5f, 0.0f ), vFloat2(24.0f, 32.0f) }

		{ vFloat3(  0.0f,  0.0f, 1.0f ), vFloat2( 0.0f,  0.0f) },
		{ vFloat3(  0.0f,  1.0f, 1.0f ), vFloat2( 0.0f, 32.0f) },
		{ vFloat3(  1.0f,  1.0f, 1.0f ), vFloat2(24.0f, 32.0f) },
		{ vFloat3(  1.0f,  0.0f, 1.0f ), vFloat2(24.0f,  0.0f) }
	};

	dx11_CreateStaticMesh(gpu_mesh_box2D, vertices, sizeof(vertices[0]), bulkof(vertices));
	// ---------------------------------------------------------------------------------------------
}

PlayerSprite::PlayerSprite() {
	xMemZero(gpu_layout_sprite);
	gpu_layout_sprite.AddVertexSlot( {
		{ "POSITION", GPU_ResourceFmt_R32G32B32_FLOAT	},
		{ "TEXCOORD", GPU_ResourceFmt_R32G32_FLOAT		}
	});

	dx11_CreateConstantBuffer(gpu_constbuf, sizeof(float2) + sizeof(int2));

	m_position = { 10, 10 };
	m_frame_id = 0;
	m_frame_timeout = 0;
}

static bool s_isAbsolute = false;

void PlayerSprite::Tick(u32 order, float dt)
{
	// Pad Input processing should be moved to Main and invoked as a "PollLocalUserInput", which in
	// turn updates the LocalUser's PlayerSprite accordingly.

	PadState state;
	KPad_GetState(state);

	g_ViewCamera.m_At += { state.axis.RStick_X * 0.05f, state.axis.RStick_Y * 0.05f };

	const float playerSpeed = 3.f;	// tiles per second

	float2 direction = {};

	if (state.buttons.DPad_Up)
		direction.y = -playerSpeed;
	else if (state.buttons.DPad_Down)
		direction.y = playerSpeed;

	if (state.buttons.DPad_Left)
		direction.x = -playerSpeed;
	else if (state.buttons.DPad_Right)
		direction.x = playerSpeed;

	if (state.axis.LStick_X < 0)
		direction.x = -playerSpeed;
	else if (state.axis.LStick_X > 0)
		direction.x = playerSpeed;

	if (state.axis.LStick_Y < 0)
		direction.y = -playerSpeed;
	else if (state.axis.LStick_Y > 0)
		direction.y = playerSpeed;

	m_position += direction * dt;

	if (direction.isEmpty()) {
		m_frame_id = 1;
	}
	else {
		m_frame_timeout -= dt;
		if (m_frame_timeout < 0) {
			m_frame_timeout += 0.15f;
			if (++m_frame_id >= 3) {
				m_frame_id = 0;
			}
		}
	}
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


void PlayerSprite::Draw(float zorder) const
{
	// TODO: most of this is generic for all sprites and should be default for all items
	// in the sprites render list.  Moreover, the API should be such that any locally-modified GPU
	// states should be restored after the sprite draw is finished.

	dx11_BindShaderVS		(g_ShaderVS_Spriter);
	dx11_BindShaderFS		(g_ShaderFS_Spriter);
	dx11_SetInputLayout		(gpu_layout_sprite);
	dx11_BindShaderResource	(tex_camel[m_frame_id], 0);
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