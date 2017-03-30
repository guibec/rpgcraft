
#include "msw-redtape.h"

#include "x-stl.h"
#include "x-string.h"
#include "x-thread.h"

#include "v-float.h"
#include "x-gpu-ifc.h"

#include <d3d11_1.h>
#include <d3dcompiler.h>
#include <directxmath.h>
#include <DirectXColors.h>

DECLARE_MODULE_NAME("dx11");

using namespace DirectX;

static const int BackBufferCount = 3;

extern HINSTANCE		g_hInst;
extern HWND				g_hWnd;

ID3D11RasterizerState*	g_RasterState[_GPU_Fill_Count_][_GPU_Cull_Count_][_GPU_Scissor_Count_];

D3D_DRIVER_TYPE         g_driverType			= D3D_DRIVER_TYPE_NULL;
D3D_FEATURE_LEVEL       g_featureLevel			= D3D_FEATURE_LEVEL_11_0;
ID3D11Device*           g_pd3dDevice			= nullptr;
ID3D11Device1*          g_pd3dDevice1			= nullptr;
ID3D11DeviceContext*    g_pImmediateContext		= nullptr;
ID3D11DeviceContext1*   g_pImmediateContext1	= nullptr;
IDXGISwapChain*         g_pSwapChain			= nullptr;
IDXGISwapChain1*        g_pSwapChain1			= nullptr;
ID3D11RenderTargetView* g_pRenderTargetView		= nullptr;

ID3D11VertexShader*     g_pVertexShader			= nullptr;
ID3D11PixelShader*      g_pPixelShader			= nullptr;
ID3D11InputLayout*      g_pVertexLayout			= nullptr;
ID3D11Buffer*           g_pConstantBuffer		= nullptr;

XMMATRIX                g_World;
XMMATRIX                g_View;
XMMATRIX                g_Projection;

int g_curBufferIdx = 0;

// * Vertex Buffers are Mostly Dynamic.
// * Use rotating buffers to avoid blocking on prev frame in order to setup new frame.
// * Index Buffers use series of "Default Layouts" which can be packed into a single buffer.

struct DynamicVertexBufferSet {

	// Vertex lists will need to be split according to their layout - namely their stride.
	// eg, if some have texture UV data while others do not.
	// It makes the most sense if the buffers are named after the vertex data types that they contain.
	// Or more generically they can be named according to the buffer stride, since the underlying data
	// is not important.

	// For now just have the Simple buffer...

	ID3D11Buffer*		Buffers[128]	= { nullptr };
};


int						g_DynVertBufferCount = 0;
DynamicVertexBufferSet  g_DynVertBuffers[BackBufferCount];


//--------------------------------------------------------------------------------------
// Helper for compiling shaders with D3DCompile
//
// With VS 11, we could load up prebuilt .cso files instead...
//--------------------------------------------------------------------------------------
HRESULT TryCompileShaderFromFile(WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut)
{
	HRESULT hr = S_OK;

	DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#ifdef _DEBUG
	// Set the D3DCOMPILE_DEBUG flag to embed debug information in the shaders.
	// Setting this flag improves the shader debugging experience, but still allows 
	// the shaders to be optimized and to run exactly the way they will run in 
	// the release configuration of this program.
	dwShaderFlags |= D3DCOMPILE_DEBUG;

	// Disable optimizations to further improve shader debugging
	dwShaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

	ID3DBlob* pErrorBlob = nullptr;
	hr = D3DCompileFromFile(szFileName, nullptr, nullptr, szEntryPoint, szShaderModel,
		dwShaderFlags, 0, ppBlobOut, &pErrorBlob);

	if (FAILED(hr)) {
		if (pErrorBlob) {
			OutputDebugStringA(reinterpret_cast<const char*>(pErrorBlob->GetBufferPointer()));
		}
	}

	if (pErrorBlob) pErrorBlob->Release();
	return hr;
}

ID3DBlob* CompileShaderFromFile(WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel)
{
	ID3DBlob* ppBlobOut;
	auto hr = TryCompileShaderFromFile(szFileName, szEntryPoint, szShaderModel, &ppBlobOut);
	if (FAILED(hr)) {
		MessageBox(nullptr,
			L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK
		);
	}

	return ppBlobOut;
}

//--------------------------------------------------------------------------------------
// Create Direct3D device and swap chain
//--------------------------------------------------------------------------------------
HRESULT InitDevice()
{
	HRESULT hr = S_OK;

	RECT rc;
	GetClientRect(g_hWnd, &rc);
	UINT width = rc.right - rc.left;
	UINT height = rc.bottom - rc.top;

	UINT createDeviceFlags = 0;
#ifdef _DEBUG
	createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	D3D_DRIVER_TYPE driverTypes[] =
	{
		D3D_DRIVER_TYPE_HARDWARE,
		D3D_DRIVER_TYPE_WARP,
		D3D_DRIVER_TYPE_REFERENCE,
	};
	UINT numDriverTypes = ARRAYSIZE(driverTypes);

	D3D_FEATURE_LEVEL featureLevels[] =
	{
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
	};
	UINT numFeatureLevels = ARRAYSIZE(featureLevels);

	for (UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++)
	{
		g_driverType = driverTypes[driverTypeIndex];
		hr = D3D11CreateDevice(nullptr, g_driverType, nullptr, createDeviceFlags, featureLevels, numFeatureLevels,
			D3D11_SDK_VERSION, &g_pd3dDevice, &g_featureLevel, &g_pImmediateContext);

		if (hr == E_INVALIDARG)
		{
			// DirectX 11.0 platforms will not recognize D3D_FEATURE_LEVEL_11_1 so we need to retry without it
			hr = D3D11CreateDevice(nullptr, g_driverType, nullptr, createDeviceFlags, &featureLevels[1], numFeatureLevels - 1,
				D3D11_SDK_VERSION, &g_pd3dDevice, &g_featureLevel, &g_pImmediateContext);
		}

		if (SUCCEEDED(hr))
			break;
	}

	if (FAILED(hr))
		return hr;

	//  * DX Docs recommend always having MSAA enabled.
	//  * Depth Bias options are pretty much jsut for shadow implementations
	//  * FrontCounterClockwise is *probably* a system-wide setting.  Can't think of a good reason to have
	//    models with mixed clockwise/counter-clockwise vertex order.  Preprocess them bitches!
	//  * Wireframe of any raster state should be created, simply for debugging purposes.
	//  * That leaves just Cull Mode and Scissor Enable as the only other wildcards.

	D3D11_RASTERIZER_DESC drd = {
		D3D11_FILL_SOLID,	//D3D11_FILL_MODE FillMode;
		D3D11_CULL_NONE,	//D3D11_CULL_MODE CullMode;
		FALSE,				//BOOL FrontCounterClockwise;
		0,					//INT DepthBias;
		0.0f,				//FLOAT DepthBiasClamp;
		0.0f,				//FLOAT SlopeScaledDepthBias;
		TRUE,				//BOOL DepthClipEnable;
		FALSE,				//BOOL ScissorEnable;
		TRUE,				//BOOL MultisampleEnable;
		FALSE				//BOOL AntialiasedLineEnable;        
	};

	for (int fill=0; fill<_GPU_Fill_Count_; ++fill) {
		switch(fill) {
			case GPU_Fill_Solid:		drd.FillMode = D3D11_FILL_SOLID;		break;
			case GPU_Fill_Wireframe:	drd.FillMode = D3D11_FILL_WIREFRAME;	break;
			default: __unreachable();
		}

		for (int cull=0; cull<_GPU_Cull_Count_; ++cull) {
			switch(cull) {
				case GPU_Cull_None:		drd.CullMode = D3D11_CULL_NONE;		break;
				case GPU_Cull_Front:	drd.CullMode = D3D11_CULL_FRONT;	break;
				case GPU_Cull_Back:		drd.CullMode = D3D11_CULL_BACK;		break;
				default: __unreachable();
			}

			for (int scissor=0; scissor<_GPU_Scissor_Count_; ++scissor) {
				drd.ScissorEnable = (scissor == GPU_Scissor_Enable) ? 1 : 0;
				hr = g_pd3dDevice->CreateRasterizerState(&drd, &g_RasterState[fill][cull][scissor]);
				bug_on( FAILED( hr ));
			}
		}
	}

	// Obtain DXGI factory from device (since we used nullptr for pAdapter above)
	IDXGIFactory1* dxgiFactory = nullptr;
	{
		IDXGIDevice* dxgiDevice = nullptr;
		hr = g_pd3dDevice->QueryInterface(__uuidof(IDXGIDevice), reinterpret_cast<void**>(&dxgiDevice));
		if (SUCCEEDED(hr))
		{
			IDXGIAdapter* adapter = nullptr;
			hr = dxgiDevice->GetAdapter(&adapter);
			if (SUCCEEDED(hr))
			{
				hr = adapter->GetParent(__uuidof(IDXGIFactory1), reinterpret_cast<void**>(&dxgiFactory));
				adapter->Release();
			}
			dxgiDevice->Release();
		}
	}
	if (FAILED(hr))
		return hr;

	// Create swap chain
	IDXGIFactory2* dxgiFactory2 = nullptr;
	hr = dxgiFactory->QueryInterface(__uuidof(IDXGIFactory2), reinterpret_cast<void**>(&dxgiFactory2));
	if (dxgiFactory2)
	{
		// DirectX 11.1 or later
		hr = g_pd3dDevice->QueryInterface(__uuidof(ID3D11Device1), reinterpret_cast<void**>(&g_pd3dDevice1));
		if (SUCCEEDED(hr))
		{
			(void)g_pImmediateContext->QueryInterface(__uuidof(ID3D11DeviceContext1), reinterpret_cast<void**>(&g_pImmediateContext1));
		}

		DXGI_SWAP_CHAIN_DESC1 sd;
		ZeroMemory(&sd, sizeof(sd));
		sd.Width                = width;
		sd.Height               = height;
		sd.Format               = DXGI_FORMAT_R8G8B8A8_UNORM;
		sd.SampleDesc.Count     = 1;
		sd.SampleDesc.Quality   = 0;
		sd.BufferUsage          = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		sd.BufferCount          = 1;

		hr = dxgiFactory2->CreateSwapChainForHwnd(g_pd3dDevice, g_hWnd, &sd, nullptr, nullptr, &g_pSwapChain1);
		if (SUCCEEDED(hr))
		{
			hr = g_pSwapChain1->QueryInterface(__uuidof(IDXGISwapChain), reinterpret_cast<void**>(&g_pSwapChain));
		}

		dxgiFactory2->Release();
	} else
	{
		// DirectX 11.0 systems
		DXGI_SWAP_CHAIN_DESC sd;
		ZeroMemory(&sd, sizeof(sd));
		sd.BufferCount          = 1;
		sd.BufferDesc.Width     = width;
		sd.BufferDesc.Height    = height;
		sd.BufferDesc.Format    = DXGI_FORMAT_R8G8B8A8_UNORM;
		sd.BufferDesc.RefreshRate.Numerator = 60;
		sd.BufferDesc.RefreshRate.Denominator = 1;
		sd.BufferUsage          = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		sd.OutputWindow         = g_hWnd;
		sd.SampleDesc.Count     = 1;
		sd.SampleDesc.Quality   = 0;
		sd.Windowed             = TRUE;

		hr = dxgiFactory->CreateSwapChain(g_pd3dDevice, &sd, &g_pSwapChain);
	}

	// Note this tutorial doesn't handle full-screen swapchains so we block the ALT+ENTER shortcut
	dxgiFactory->MakeWindowAssociation(g_hWnd, DXGI_MWA_NO_ALT_ENTER);

	dxgiFactory->Release();

	if (FAILED(hr))
		return hr;

	// Create a render target view
	ID3D11Texture2D* pBackBuffer = nullptr;
	hr = g_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&pBackBuffer));
	bug_on(FAILED(hr));

	hr = g_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &g_pRenderTargetView);
	pBackBuffer->Release();
	bug_on(FAILED(hr));

	g_pImmediateContext->OMSetRenderTargets(1, &g_pRenderTargetView, nullptr);

	// Setup the viewport
	D3D11_VIEWPORT vp;
	vp.Width	= (float)width;
	vp.Height	= (float)height;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	g_pImmediateContext->RSSetViewports(1, &vp);

	// SAMPLE LOADS SHADER(s) HERE.

	// Compile the vertex shader
	ID3DBlob* pVSBlob = CompileShaderFromFile(L"Tutorial04.fx", "VS", "vs_4_0");

	// Create the vertex shader
	hr = g_pd3dDevice->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), nullptr, &g_pVertexShader);
	if (FAILED(hr))
	{
		pVSBlob->Release();
		return hr;
	}

	// Define the input layout
	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION",	0, DXGI_FORMAT_R32G32B32_FLOAT,		0, 0,	D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR",		0, DXGI_FORMAT_R32G32B32A32_FLOAT,	0, 12,	D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	UINT numElements = ARRAYSIZE(layout);

	// Create the input layout
	hr = g_pd3dDevice->CreateInputLayout(layout, numElements, pVSBlob->GetBufferPointer(),
		pVSBlob->GetBufferSize(), &g_pVertexLayout
	);
	pVSBlob->Release();
	if (FAILED(hr))
		return hr;

	// Set the input layout
	g_pImmediateContext->IASetInputLayout(g_pVertexLayout);

	// Compile the pixel shader
	ID3DBlob* pPSBlob = CompileShaderFromFile(L"Tutorial04.fx", "PS", "ps_4_0");

	// Create the pixel shader
	hr = g_pd3dDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, &g_pPixelShader);
	pPSBlob->Release();
	if (FAILED(hr))
		return hr;


	XMVECTOR Eye	= XMVectorSet( 0.0f, 1.0f, -5.0f, 0.0f );
	XMVECTOR At		= XMVectorSet( 0.0f, 1.0f, 0.0f, 0.0f );
	XMVECTOR Up		= XMVectorSet( 0.0f, 1.0f, 0.0f, 0.0f );

	g_World			= XMMatrixIdentity();
	g_View			= XMMatrixLookAtLH( Eye, At, Up );
	g_Projection	= XMMatrixPerspectiveFovLH( XM_PIDIV2, width / (FLOAT)height, 0.01f, 100.0f );

	return hr;
}

void dx11_SetVertexBuffer( int bufferId, int shaderSlot, int _stride, int _offset)
{
	uint stride = _stride;
	uint offset = _offset;

	g_pImmediateContext->IASetVertexBuffers(0, 1, &g_DynVertBuffers[g_curBufferIdx].Buffers[bufferId], &stride, &offset);
}

void dx11_SetIndexBuffer(GPU_IndexBuffer indexBuffer, int bitsPerIndex, int offset)
{
	DXGI_FORMAT format;
	switch (bitsPerIndex) {
		case 8:		format = DXGI_FORMAT_R8_UINT;		break;
		case 16:	format = DXGI_FORMAT_R16_UINT;		break;
		case 32:	format = DXGI_FORMAT_R32_UINT;		break;
		default:	unreachable("Invalid parameter 'bitsPerindex=%d'", bitsPerIndex);
	}
	g_pImmediateContext->IASetIndexBuffer( (ID3D11Buffer*)indexBuffer.m_driverData, format, offset);
}


void dx11_UploadDynamicBufferData(int bufferIdx, void* srcData, int sizeInBytes)
{
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	xMemZero(mappedResource);

	auto*   simple      = g_DynVertBuffers[g_curBufferIdx].Buffers[bufferIdx];

	g_pImmediateContext->Map(simple, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	xMemCopy(mappedResource.pData, srcData, sizeInBytes);
	g_pImmediateContext->Unmap(simple, 0);
}

int dx11_CreateDynamicVertexBuffer(int bufferSizeInBytes)
{
	int bufferIdx = g_DynVertBufferCount;
	for (int i=0; i<BackBufferCount; ++i) {
		D3D11_BUFFER_DESC bd;
		xMemZero(bd);
		bd.Usage			= D3D11_USAGE_DYNAMIC;
		bd.ByteWidth		= bufferSizeInBytes;
		bd.BindFlags		= D3D11_BIND_VERTEX_BUFFER;
		bd.CPUAccessFlags	= D3D11_CPU_ACCESS_WRITE;

		auto hr = g_pd3dDevice->CreateBuffer(&bd, nullptr, &g_DynVertBuffers[i].Buffers[bufferIdx]);
		bug_on (FAILED(hr));
	}
	g_DynVertBufferCount += 1;
	return bufferIdx;
}

GPU_IndexBuffer dx11_CreateIndexBuffer(void* indexBuffer, int bufferSize)
{
	D3D11_SUBRESOURCE_DATA InitData;
	D3D11_BUFFER_DESC bd;

	xMemZero(bd);
	bd.Usage			= D3D11_USAGE_DEFAULT;
	bd.ByteWidth		= bufferSize;
	bd.BindFlags		= D3D11_BIND_INDEX_BUFFER;
	bd.CPUAccessFlags	= 0;

	xMemZero(InitData);
	InitData.pSysMem	= indexBuffer;

	ID3D11Buffer* handle;
	auto hr = g_pd3dDevice->CreateBuffer( &bd, &InitData, &handle );
	bug_on (FAILED(hr));

	return (GPU_IndexBuffer)handle;
}

void CleanupDevice()
{
	// ... one of these isn't like all the others... !!!

	if (g_pImmediateContext)	g_pImmediateContext		->ClearState();

	if (g_pSwapChain1)			g_pSwapChain1			->Release();
	if (g_pSwapChain)			g_pSwapChain			->Release();
	if (g_pImmediateContext1)	g_pImmediateContext1	->Release();
	if (g_pImmediateContext)	g_pImmediateContext		->Release();
	if (g_pd3dDevice1)			g_pd3dDevice1			->Release();
	if (g_pd3dDevice)			g_pd3dDevice			->Release();
}

void dx11_BackbufferSwap()
{
	g_pSwapChain->Present(0, 0);
	g_curBufferIdx = (g_curBufferIdx+1) % BackBufferCount;
}

void dx11_SetPrimType(GpuPrimitiveType primType)
{
	D3D_PRIMITIVE_TOPOLOGY dxPrimTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	switch(primType) {
		case GPU_PRIM_POINTLIST		: dxPrimTopology = D3D_PRIMITIVE_TOPOLOGY_POINTLIST		;	break;
		case GPU_PRIM_LINELIST		: dxPrimTopology = D3D_PRIMITIVE_TOPOLOGY_LINELIST		;	break;
		case GPU_PRIM_LINESTRIP		: dxPrimTopology = D3D_PRIMITIVE_TOPOLOGY_LINESTRIP		;	break;
		case GPU_PRIM_TRIANGLELIST	: dxPrimTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST	;	break;
		case GPU_PRIM_TRIANGLESTRIP	: dxPrimTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP	;	break;

		default: unreachable("");
	}
	
	g_pImmediateContext->IASetPrimitiveTopology(dxPrimTopology);
}

void dx11_SetRasterState(GpuRasterFillMode fill, GpuRasterCullMode cull, GpuRasterScissorMode scissor)
{
	if (g_gpu_ForceWireframe) {
		fill = GPU_Fill_Wireframe;
	}
	g_pImmediateContext->RSSetState(g_RasterState[fill][cull][scissor]);
}
