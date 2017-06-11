
#include "x-types.h"
#include "x-stl.h"
#include "x-assertion.h"
#include "x-string.h"
#include "x-thread.h"
#include "x-gpu-ifc.h"
#include "x-gpu-colors.h"

#include "x-png-decode.h"

#include "Bezier2D.h"

#include "Bezier2d.inl"


DECLARE_MODULE_NAME("main");

GPU_VertexBuffer		g_mesh_box2D;
GPU_IndexBuffer			g_idx_box2D;
GPU_ShaderVS			g_ShaderVS;
GPU_ShaderFS			g_ShaderFS;

bool					g_gpu_ForceWireframe	= false;

GPU_TextureResource2D	tex_floor;
GPU_TextureResource2D	tex_terrain;


static const int TerrainTileW = 256;
static const int TerrainTileH = 256;

float s_ProcTerrain_Height[TerrainTileW][TerrainTileH];

void ProcGenTerrain()
{
	// Generate a heightmap!
	
	// Base heightmap on several interfering waves.

	for (int y=0; y<TerrainTileH; ++y) {
		for (int x=0; x<TerrainTileW; ++x) {
			float continental_wave = std::sinf((y+x) * 0.006);
			s_ProcTerrain_Height[y][x] = continental_wave; // * 32767.0f;
		}
	}
}


struct TileMapVertex {
	vFloat3		xyz;
	vFloat2		uv;
};

void Render()
{
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

	dx11_BindShaderVS(g_ShaderVS);
	dx11_BindShaderFS(g_ShaderFS);

	dx11_SetPrimType(GPU_PRIM_TRIANGLELIST);
	dx11_BindShaderResource(tex_floor, 0);

	//g_pImmediateContext->DrawIndexed((numVertexesPerCurve*4), 0,  0);
	//g_pImmediateContext->VSSetConstantBuffers(0, 1, &g_pConstantBuffer);


	dx11_SetVertexBuffer(g_mesh_box2D, 0, sizeof(TileMapVertex), 0);
	dx11_SetIndexBuffer(g_idx_box2D, 16, 0);
	dx11_DrawIndexed(6, 0,  0);

	//g_pSwapChain->Present(1, DXGI_SWAP_EFFECT_SEQUENTIAL);

	dx11_BackbufferSwap();
	xThreadSleep(20);
}

void DoGameInit()
{
	xBitmapData  pngtex;
	png_LoadFromFile(pngtex, "..\\rpg_maker_vx__modernrtp_tilea2_by_painhurt-d3f7rwg.png");

	dx11_CreateTexture2D(tex_floor, pngtex.buffer.GetPtr(), pngtex.width, pngtex.height, GPU_ResourceFmt_R8G8B8A8_UNORM);

	ProcGenTerrain();
	dx11_CreateTexture2D(tex_terrain, s_ProcTerrain_Height, TerrainTileW, TerrainTileH, GPU_ResourceFmt_R32_FLOAT);

	extern void dx11_SetInputLayout();
	dx11_LoadShaderVS(g_ShaderVS, "HeightMappedQuad.fx", "VS");
	dx11_LoadShaderFS(g_ShaderFS, "HeightMappedQuad.fx", "PS");
	dx11_SetInputLayout();

TileMapVertex vertices[] =
{
	{ vFloat3( -0.4f,  0.5f, 0.5f ), vFloat2(0.0f, 0.0f) },
	{ vFloat3( -0.4f, -0.5f, 0.5f ), vFloat2(0.0f, 1.0f) },
	{ vFloat3(  0.4f, -0.5f, 0.5f ), vFloat2(1.0f, 1.0f) },
	{ vFloat3(  0.4f,  0.5f, 0.5f ), vFloat2(1.0f, 0.0f) }

	//{ vFloat3( -1.0f,  1.0f, 0.5f ), vFloat2(0.0f, 0.0f) },
	//{ vFloat3( -1.0f, -1.0f, 0.5f ), vFloat2(0.0f, 1.0f) },
	//{ vFloat3(  1.0f, -1.0f, 0.5f ), vFloat2(1.0f, 1.0f) },
	//{ vFloat3(  1.0f,  1.0f, 0.5f ), vFloat2(1.0f, 0.0f) }
};

	s16 indices_box[] = {
		0,1,3,
		1,3,2
	};

	g_mesh_box2D = dx11_CreateStaticMesh(vertices, sizeof(vertices[0]), bulkof(vertices));
	g_idx_box2D  = dx11_CreateIndexBuffer(indices_box, 6*2);
}
