
#include "PCH-rpgcraft.h"
#include "Sprites.h"

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
		// XYZ is in tiles .. where Z is expected to be 1.0f

		{ vFloat3( -0.5f,  0.5f, 0.0f ), vFloat2( 0.0f, 32.0f) },
		{ vFloat3( -0.5f, -0.5f, 0.0f ), vFloat2( 0.0f, 64.0f) },
		{ vFloat3(  0.5f, -0.5f, 0.0f ), vFloat2(24.0f, 64.0f) },
		{ vFloat3(  0.5f,  0.5f, 0.0f ), vFloat2(24.0f, 32.0f) }
	};

	dx11_CreateStaticMesh(gpu_mesh_box2D, vertices, sizeof(vertices[0]), bulkof(vertices));
	// ---------------------------------------------------------------------------------------------
}

void PlayerSprite::Tick()
{
}

void PlayerSprite::Draw() const
{
	dx11_BindShaderVS		(g_ShaderVS_Spriter);
	dx11_BindShaderFS		(g_ShaderFS_Spriter);
	dx11_SetInputLayout		(gpu_layout_sprite);
	dx11_BindShaderResource	(tex_chars, 0);
	dx11_SetVertexBuffer	(gpu_mesh_box2D, 0, sizeof(TileMapVertex), 0);
	dx11_SetIndexBuffer		(g_idx_box2D, 16, 0);

	dx11_DrawIndexed(6, 0,  0);
}

void UniformMeshes_InitGlobalResources() {
	dx11_CreateIndexBuffer(g_idx_box2D, g_ind_UniformQuad, sizeof(g_ind_UniformQuad));
}