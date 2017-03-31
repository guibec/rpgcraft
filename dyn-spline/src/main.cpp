
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

	dx11_SetIndexBuffer(g_IndexBuffer, 16, 0);
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
}
