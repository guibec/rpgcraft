
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

static const int const_zval = 0.1f;


int						g_VertexBufferId;
GPU_IndexBuffer			g_IndexBuffer;
GPU_VertexBuffer		g_mesh_box2D;
GPU_IndexBuffer			g_idx_box2D;
GPU_ShaderVS			g_ShaderVS;
GPU_ShaderFS			g_ShaderFS;

bool					g_gpu_ForceWireframe	= false;

GPU_TextureResource2D	tex_floor;
GPU_TextureResource2D	tex_terrain;

static const int		numStepsPerCurve		= 10;
static const int		numTrisPerCurve		    =  numStepsPerCurve;
static const int		numVertexesPerCurve	    = (numTrisPerCurve * 3);
static const int		SimpleVertexBufferSize  = (numTrisPerCurve*4) + 1;


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


void PopulateIndices_TriFan(s16* dest, int& iidx, int& vertexIdx, int numSubdivs)
{
	for (int idx = 0; idx < numSubdivs; ++idx)
	{
		// non-interpolated (flat shading) determines color according the first vertex.
		// To allow for non-solid colorization of the objectm make sure the first vertex
		// is *not* the center point:

		dest[iidx + 0] = vertexIdx + 0;
		dest[iidx + 1] = 0;
		dest[iidx + 2] = vertexIdx + 1;
		vertexIdx   += 1;
		iidx        += 3;
	}
}

struct TileMapVertex {
	vFloat3		xyz;
	vFloat2		uv;
};

// TripStrip is natively supported by all GPUs, so providing an index list is not necessary.
// This function may be useful for some fancier mesh types though, where it consists of some
// mix of fan and strip triangles, etc...
void PopulateIndices_TriStrip(s16* dest, int& iidx, int& vertexIdx, int numSubdivs)
{
	// Expected vertex layout:
	//   vidx 0   - Edge 0
	//   vidx 1,2 - Center, Edge 1
	//   vidx 3,4 - Center, Edge 2
	//   vidx 5,6 - Center, Edge 3

	bug_on(vertexIdx==0, "vertexIdx must be at least 1; vertexIdx-1 should be first vertex along outside edge.");

	for (int idx = 0; idx < numSubdivs; ++idx)
	{
		dest[iidx + 0] = vertexIdx - 1;
		dest[iidx + 1] = vertexIdx;
		dest[iidx + 2] = vertexIdx + 1;
		vertexIdx   += 2;
		iidx        += 3;
	}
}

void Render()
{
	const vFloat2 top1[4] = {
		{  -0.5f,  -0.5f },
		{  -0.2f,  -0.3f },		// control point
		{	0.3f,  -0.6f },		// control point
		{	0.5f,  -0.5f },
	};

	const vFloat2 left1[4] = {
		{   0.5f,  -0.5f },
		{	0.8f,  -0.2f },		// control point
		{	0.8f,   0.2f },		// control point
		{   0.7f,   0.6f },
	};

	const vFloat2 bottom1[4] = {
		{	0.7f,   0.6f },
		{	0.3f,   0.3f },		// control point
		{  -0.2f,   0.9f },		// control point
		{  -0.5f,   0.5f },
	};

	const vFloat2 right1[4] = {
		{  -0.5f,   0.5f },
		{  -0.8f,   0.2f },		// control point
		{  -0.8f,  -0.2f },		// control point
		{  -0.5f,  -0.5f },
	};


	// --------------------------------------------------------------------------------------

	const vFloat2 top2[4] = {
		{  -0.3f,  -0.5f },
		{  -0.2f,  -0.3f },		// control point
		{	0.3f,  -0.6f },		// control point
		{	0.5f,  -0.3f },
	};

	const vFloat2 left2[4] = {
		{   0.5f,  -0.3f },
		{	0.8f,  -0.4f },		// control point
		{	0.8f,   0.2f },		// control point
		{   0.5f,   0.5f },
	};

	const vFloat2 bottom2[4] = {
		{	0.5f,   0.5f },
		{	0.3f,   0.3f },		// control point
		{  -0.2f,   0.7f },		// control point
		{  -0.5f,   0.5f },
	};

	const vFloat2 right2[4] = {
		{  -0.5f,   0.5f },
		{  -0.8f,   0.2f },		// control point
		{  -0.9f,  -0.2f },		// control point
		{  -0.3f,  -0.5f },
	};

	static float animPos = 0.0f;

	animPos += 0.06f;
	animPos = std::fmodf(animPos, 3.14159f * 4);

	float apos = (1.0f + std::sinf(animPos)) * 0.5f;

#define lerp(a,b)		((a) + (((b)-(a))* (apos)))

#define lerpBezier(varA, varB)	{ \
	{ ((varA)[0].x + (((varB)[0].x-(varA)[0].x) * (apos))), ((varA)[0].y + (((varB)[0].y-(varA)[0].y) * (apos))) },		\
	{ ((varA)[1].x + (((varB)[1].x-(varA)[1].x) * (apos))), ((varA)[1].y + (((varB)[1].y-(varA)[1].y) * (apos))) },		\
	{ ((varA)[2].x + (((varB)[2].x-(varA)[2].x) * (apos))), ((varA)[2].y + (((varB)[2].y-(varA)[2].y) * (apos))) },		\
	{ ((varA)[3].x + (((varB)[3].x-(varA)[3].x) * (apos))), ((varA)[3].y + (((varB)[3].y-(varA)[3].y) * (apos))) },		\
}

	vFloat2 top		[4] = lerpBezier(top1,		top2	);
	vFloat2 left	[4] = lerpBezier(left1,		left2	);
	vFloat2 bottom	[4]	= lerpBezier(bottom1,	bottom2	);
	vFloat2 right	[4]	= lerpBezier(right1,	right2	);

	SimpleVertex vertices[SimpleVertexBufferSize];

	vFloat2		center	= vFloat2(0.0f,		0.0f);

	vertices[0].Pos = vFloat3(center, const_zval);

	VertexBufferState<SimpleVertex> vstate = { 1, vertices };

	SubDiv_BezierFan(vstate, numStepsPerCurve, center, top);
	SubDiv_BezierFan(vstate, numStepsPerCurve, center, left);
	SubDiv_BezierFan(vstate, numStepsPerCurve, center, bottom);
	SubDiv_BezierFan(vstate, numStepsPerCurve, center, right);

	assume(vstate.m_vidx <= bulkof(vertices));

	// ------------------------------------------------------------------------------------------
	// Apply Diagnostic color for visualizing geometry...

	if (1) {
		static float colorYay = 0.0f;
		float colorIdx = colorYay; //0.0f;
		foreach( auto& vert, vertices ) {
			vert.Color = vFloat4( colorIdx, 0.0f, 0.0f, 1.0f );
			colorIdx  += 0.05f;
			colorIdx   = std::fmodf(colorIdx, 1.0f);
		}

		colorYay += 0.01f;
		vertices[0].Color.x = 0.0f;
		vertices[0].Color.y = 0.0f;
		vertices[0].Color.z = 0.0f;
	}


	// Clear the back buffer
	dx11_ClearRenderTarget(g_gpu_BackBuffer, GPU_Colors::MidnightBlue);

	// ------------------------------------------------------------------------------------------
	// Update Spline Vertex Information

	if (g_VertexBufferId) {
		dx11_UploadDynamicBufferData(g_VertexBufferId, vertices, sizeof(vertices));
	}

	// ------------------------------------------------------------------------------------------


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

	dx11_SetIndexBuffer(g_IndexBuffer, 16, 0);
	dx11_SetVertexBuffer(g_VertexBufferId, 0, sizeof(SimpleVertex), 0);
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

void InitSplineTest()
{
	s16	   indices [(numVertexesPerCurve*4)];

	int iidx = 0;
	int vidx = 1;

	PopulateIndices_TriFan(indices, iidx, vidx, numStepsPerCurve);
	PopulateIndices_TriFan(indices, iidx, vidx, numStepsPerCurve);
	PopulateIndices_TriFan(indices, iidx, vidx, numStepsPerCurve);
	PopulateIndices_TriFan(indices, iidx, vidx, numStepsPerCurve);

	// Close the patch by creating triangle between last vertex along the spline and first one.

	indices[(iidx-3)+1] = 0;
	indices[(iidx-3)+0] = vidx-1;
	indices[(iidx-3)+2] = 1;

	assume(vidx <= SimpleVertexBufferSize);
	assume(iidx <= bulkof(indices));

	g_VertexBufferId	= dx11_CreateDynamicVertexBuffer(sizeof(SimpleVertex) * SimpleVertexBufferSize);
	g_IndexBuffer		= dx11_CreateIndexBuffer(indices, sizeof(indices));
}

#include "ajek-script.h"

void LoadLocalConfig()
{
	AjekScript_InitModuleList();

	auto& env	= AjekScriptEnv_Get(ScriptEnv_AppConfig);
	auto* L		= env.getLuaState();

	env.LoadModule("config-local.lua");

	auto path  = env.glob_get_string("ScriptsPath");
	
	log_host( "Path Test = %s", path.c_str() );
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
