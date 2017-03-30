
#include "x-types.h"
#include "x-stl.h"
#include "x-assertion.h"
#include "x-string.h"
#include "x-thread.h"
#include "x-gpu-ifc.h"

#include "Bezier2D.h"

#include <windows.h>
#include <d3d11_1.h>
#include <DirectXColors.h>

#include "Bezier2d.inl"

using namespace DirectX;

static const int const_zval = 0.1f;

extern ID3D11DeviceContext*    g_pImmediateContext;
extern ID3D11RenderTargetView* g_pRenderTargetView;
extern ID3D11VertexShader*     g_pVertexShader;
extern ID3D11PixelShader*      g_pPixelShader;

int						g_VertexBufferId;
GPU_IndexBuffer         g_IndexBuffer;

bool					g_gpu_ForceWireframe	= false;


static const int s_perlin_permutation[512] = {
	151,160,137,91,90,15,
	131,13,201,95,96,53,194,233,7,225,140,36,103,30,69,142,8,99,37,240,21,10,23,
	190, 6,148,247,120,234,75,0,26,197,62,94,252,219,203,117,35,11,32,57,177,33,
	88,237,149,56,87,174,20,125,136,171,168, 68,175,74,165,71,134,139,48,27,166,
	77,146,158,231,83,111,229,122,60,211,133,230,220,105,92,41,55,46,245,40,244,
	102,143,54, 65,25,63,161, 1,216,80,73,209,76,132,187,208, 89,18,169,200,196,
	135,130,116,188,159,86,164,100,109,198,173,186, 3,64,52,217,226,250,124,123,
	5,202,38,147,118,126,255,82,85,212,207,206,59,227,47,16,58,17,182,189,28,42,
	223,183,170,213,119,248,152, 2,44,154,163, 70,221,153,101,155,167, 43,172,9,
	129,22,39,253, 19,98,108,110,79,113,224,232,178,185, 112,104,218,246,97,228,
	251,34,242,193,238,210,144,12,191,179,162,241, 81,51,145,235,249,14,239,107,
	49,192,214, 31,181,199,106,157,184, 84,204,176,115,121,50,45,127, 4,150,254,
	138,236,205,93,222,114,67,29,24,72,243,141,128,195,78,66,215,61,156,180,

	151,160,137,91,90,15,
	131,13,201,95,96,53,194,233,7,225,140,36,103,30,69,142,8,99,37,240,21,10,23,
	190, 6,148,247,120,234,75,0,26,197,62,94,252,219,203,117,35,11,32,57,177,33,
	88,237,149,56,87,174,20,125,136,171,168, 68,175,74,165,71,134,139,48,27,166,
	77,146,158,231,83,111,229,122,60,211,133,230,220,105,92,41,55,46,245,40,244,
	102,143,54, 65,25,63,161, 1,216,80,73,209,76,132,187,208, 89,18,169,200,196,
	135,130,116,188,159,86,164,100,109,198,173,186, 3,64,52,217,226,250,124,123,
	5,202,38,147,118,126,255,82,85,212,207,206,59,227,47,16,58,17,182,189,28,42,
	223,183,170,213,119,248,152, 2,44,154,163, 70,221,153,101,155,167, 43,172,9,
	129,22,39,253, 19,98,108,110,79,113,224,232,178,185, 112,104,218,246,97,228,
	251,34,242,193,238,210,144,12,191,179,162,241, 81,51,145,235,249,14,239,107,
	49,192,214, 31,181,199,106,157,184, 84,204,176,115,121,50,45,127, 4,150,254,
	138,236,205,93,222,114,67,29,24,72,243,141,128,195,78,66,215,61,156,180
};

static const int    numStepsPerCurve		= 10;
static const int    numTrisPerCurve		    =  numStepsPerCurve;
static const int    numVertexesPerCurve	    = (numTrisPerCurve * 3);
static const int    SimpleVertexBufferSize  = (numTrisPerCurve*4) + 1;


void PopulateIndices_TriFan(s16* dest, int& iidx, int& vertexIdx, int numSubdivs)
{
	for (int idx = 0; idx < numSubdivs; ++idx)
	{
		dest[iidx + 1] = 0;
		dest[iidx + 0] = vertexIdx + 0;
		dest[iidx + 2] = vertexIdx + 1;
		vertexIdx   += 1;
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
	g_pImmediateContext->ClearRenderTargetView(g_pRenderTargetView, Colors::MidnightBlue);

	// ------------------------------------------------------------------------------------------
	// Update Spline Vertex Information

	dx11_UploadDynamicBufferData(g_VertexBufferId, vertices, sizeof(vertices));

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

	dx11_SetVertexBuffer(g_VertexBufferId, 0, sizeof(SimpleVertex), 0);
	dx11_SetPrimType(GPU_PRIM_TRIANGLELIST);

	//g_pImmediateContext->VSSetConstantBuffers(0, 1, &g_pConstantBuffer);

	g_pImmediateContext->VSSetShader(g_pVertexShader, nullptr, 0);
	g_pImmediateContext->PSSetShader(g_pPixelShader, nullptr, 0);
	g_pImmediateContext->DrawIndexed((numVertexesPerCurve*4), 0,  0);

	//g_pSwapChain->Present(1, DXGI_SWAP_EFFECT_SEQUENTIAL);

	dx11_BackbufferSwap();
	xThreadSleep(20);
}

void DoGameInit()
{
	s16	   indices [(numVertexesPerCurve*4)];

	int iidx = 0;
	int vidx = 1;

	PopulateIndices_TriFan(indices, iidx, vidx, numStepsPerCurve);
	PopulateIndices_TriFan(indices, iidx, vidx, numStepsPerCurve);
	PopulateIndices_TriFan(indices, iidx, vidx, numStepsPerCurve);
	PopulateIndices_TriFan(indices, iidx, vidx, numStepsPerCurve);

	// Close the patch by creating triable between last vertex along the spline and first one.

	indices[(iidx-3)+1] = 0;
	indices[(iidx-3)+0] = vidx-1;
	indices[(iidx-3)+2] = 1;

	assume(vidx <= SimpleVertexBufferSize);
	assume(iidx <= bulkof(indices));

	g_VertexBufferId	= dx11_CreateDynamicVertexBuffer(sizeof(SimpleVertex) * SimpleVertexBufferSize);
	g_IndexBuffer		= dx11_CreateIndexBuffer(indices, sizeof(indices));

	// Set index buffer
	dx11_SetIndexBuffer(g_IndexBuffer, 16, 0);
}
