
#include "msw-redtape.h"

#include "x-stl.h"
#include "x-string.h"
#include "x-thread.h"

#include "v-float.h"
#include "x-gpu-ifc.h"
#include "x-ThrowContext.h"

#include <d3d11_1.h>
#include <d3dcompiler.h>
#include <directxmath.h>
#include <DirectXColors.h>

#include <wincodec.h>

DECLARE_MODULE_NAME("dx11");

#include "x-MemCopy.inl"

DECLARE_MODULE_THROW(xThrowModule_GPU);		// enables use of throw_abort() macro


#define ptr_cast		reinterpret_cast

using namespace DirectX;

static const int BackBufferCount = 3;

extern HINSTANCE		g_hInst;
extern HWND				g_hWnd;

int2 g_backbuffer_size_pix = {0, 0};


static ID3D11RasterizerState*	g_RasterState[_GPU_Fill_Count_][_GPU_Cull_Count_][_GPU_Scissor_Count_] = {};

static D3D_DRIVER_TYPE			g_driverType			= D3D_DRIVER_TYPE_NULL;
static D3D_FEATURE_LEVEL		g_featureLevel			= D3D_FEATURE_LEVEL_11_0;
static ID3D11Device*			g_pd3dDevice			= nullptr;
static ID3D11Device1*			g_pd3dDevice1			= nullptr;
static ID3D11DeviceContext*		g_pImmediateContext		= nullptr;
static ID3D11DeviceContext1*	g_pImmediateContext1	= nullptr;
static IDXGISwapChain*			g_pSwapChain			= nullptr;
static IDXGISwapChain1*			g_pSwapChain1			= nullptr;


struct ConstantBuffer
{
	XMMATRIX			World;
	XMMATRIX			View;
	XMMATRIX			Projection;
};

XMMATRIX                g_World;
XMMATRIX                g_View;
XMMATRIX                g_Projection;

GPU_RenderTarget		g_gpu_BackBuffer;
int						g_curBufferIdx = 0;

// * Vertex Buffers are Mostly Dynamic.
// * Use rotating buffers to avoid blocking on prev frame in order to setup new frame.
// * Index Buffers use series of "Default Layouts" which can be packed into a single buffer.

enum DynBufferType {
	DynBuffer_Free,			// not allocated
	DynBuffer_Vertex,
	DynBuffer_Input,
	DynBuffer_Resource,		// shader / texture resource
};

const char* enumToString(const DynBufferType& id)
{
	switch(id) {
		CaseReturnString(DynBuffer_Free			);
		CaseReturnString(DynBuffer_Vertex		);
		CaseReturnString(DynBuffer_Input		);
		CaseReturnString(DynBuffer_Resource		);
		default:	break;
	}

	unreachable_qa("Invalid or unknown DynBufferType=%d", id);
	return "unknown";
}


struct DynBufferItem
{
	ID3D11Buffer*			m_dx11_buffer;
	DynBufferType			m_type;
};

//int							g_DynVertBufferCount = 0;
DynBufferItem				g_DynVertBuffers[BackBufferCount][256];

ID3D11SamplerState*			m_pTextureSampler = nullptr;


DXGI_FORMAT get_DXGI_Format(GPU_ResourceFmt bitmapFmt)
{
	switch(bitmapFmt)
	{
		case     GPU_ResourceFmt_R32G32B32A32_TYPELESS     :  return DXGI_FORMAT_R32G32B32A32_TYPELESS       ;
		case     GPU_ResourceFmt_R32G32B32A32_FLOAT        :  return DXGI_FORMAT_R32G32B32A32_FLOAT          ;
		case     GPU_ResourceFmt_R32G32B32A32_UINT         :  return DXGI_FORMAT_R32G32B32A32_UINT           ;
		case     GPU_ResourceFmt_R32G32B32A32_SINT         :  return DXGI_FORMAT_R32G32B32A32_SINT           ;
		case     GPU_ResourceFmt_R32G32B32_TYPELESS        :  return DXGI_FORMAT_R32G32B32_TYPELESS          ;
		case     GPU_ResourceFmt_R32G32B32_FLOAT           :  return DXGI_FORMAT_R32G32B32_FLOAT             ;
		case     GPU_ResourceFmt_R32G32B32_UINT            :  return DXGI_FORMAT_R32G32B32_UINT              ;
		case     GPU_ResourceFmt_R32G32B32_SINT            :  return DXGI_FORMAT_R32G32B32_SINT              ;
		case     GPU_ResourceFmt_R16G16B16A16_TYPELESS     :  return DXGI_FORMAT_R16G16B16A16_TYPELESS       ;
		case     GPU_ResourceFmt_R16G16B16A16_FLOAT        :  return DXGI_FORMAT_R16G16B16A16_FLOAT          ;
		case     GPU_ResourceFmt_R16G16B16A16_UNORM        :  return DXGI_FORMAT_R16G16B16A16_UNORM          ;
		case     GPU_ResourceFmt_R16G16B16A16_UINT         :  return DXGI_FORMAT_R16G16B16A16_UINT           ;
		case     GPU_ResourceFmt_R16G16B16A16_SNORM        :  return DXGI_FORMAT_R16G16B16A16_SNORM          ;
		case     GPU_ResourceFmt_R16G16B16A16_SINT         :  return DXGI_FORMAT_R16G16B16A16_SINT           ;
		case     GPU_ResourceFmt_R32G32_TYPELESS           :  return DXGI_FORMAT_R32G32_TYPELESS             ;
		case     GPU_ResourceFmt_R32G32_FLOAT              :  return DXGI_FORMAT_R32G32_FLOAT                ;
		case     GPU_ResourceFmt_R32G32_UINT               :  return DXGI_FORMAT_R32G32_UINT                 ;
		case     GPU_ResourceFmt_R32G32_SINT               :  return DXGI_FORMAT_R32G32_SINT                 ;
		case     GPU_ResourceFmt_R32G8X24_TYPELESS         :  return DXGI_FORMAT_R32G8X24_TYPELESS           ;
		case     GPU_ResourceFmt_D32_FLOAT_S8X24_UINT      :  return DXGI_FORMAT_D32_FLOAT_S8X24_UINT        ;
		case     GPU_ResourceFmt_R32_FLOAT_X8X24_TYPELESS  :  return DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS    ;
		case     GPU_ResourceFmt_X32_TYPELESS_G8X24_UINT   :  return DXGI_FORMAT_X32_TYPELESS_G8X24_UINT     ;
		case     GPU_ResourceFmt_R10G10B10A2_TYPELESS      :  return DXGI_FORMAT_R10G10B10A2_TYPELESS        ;
		case     GPU_ResourceFmt_R10G10B10A2_UNORM         :  return DXGI_FORMAT_R10G10B10A2_UNORM           ;
		case     GPU_ResourceFmt_R10G10B10A2_UINT          :  return DXGI_FORMAT_R10G10B10A2_UINT            ;
		case     GPU_ResourceFmt_R8G8B8A8_TYPELESS         :  return DXGI_FORMAT_R8G8B8A8_TYPELESS           ;
		case     GPU_ResourceFmt_R8G8B8A8_UNORM            :  return DXGI_FORMAT_R8G8B8A8_UNORM              ;
		case     GPU_ResourceFmt_R8G8B8A8_UINT             :  return DXGI_FORMAT_R8G8B8A8_UINT               ;
		case     GPU_ResourceFmt_R8G8B8A8_SNORM            :  return DXGI_FORMAT_R8G8B8A8_SNORM              ;
		case     GPU_ResourceFmt_R8G8B8A8_SINT             :  return DXGI_FORMAT_R8G8B8A8_SINT               ;
		case     GPU_ResourceFmt_R16G16_TYPELESS           :  return DXGI_FORMAT_R16G16_TYPELESS             ;
		case     GPU_ResourceFmt_R16G16_FLOAT              :  return DXGI_FORMAT_R16G16_FLOAT                ;
		case     GPU_ResourceFmt_R16G16_UNORM              :  return DXGI_FORMAT_R16G16_UNORM                ;
		case     GPU_ResourceFmt_R16G16_UINT               :  return DXGI_FORMAT_R16G16_UINT                 ;
		case     GPU_ResourceFmt_R16G16_SNORM              :  return DXGI_FORMAT_R16G16_SNORM                ;
		case     GPU_ResourceFmt_R16G16_SINT               :  return DXGI_FORMAT_R16G16_SINT                 ;
		case     GPU_ResourceFmt_R32_TYPELESS              :  return DXGI_FORMAT_R32_TYPELESS                ;
		case     GPU_ResourceFmt_D32_FLOAT                 :  return DXGI_FORMAT_D32_FLOAT                   ;
		case     GPU_ResourceFmt_R32_FLOAT                 :  return DXGI_FORMAT_R32_FLOAT                   ;
		case     GPU_ResourceFmt_R32_UINT                  :  return DXGI_FORMAT_R32_UINT                    ;
		case     GPU_ResourceFmt_R32_SINT                  :  return DXGI_FORMAT_R32_SINT                    ;
		case     GPU_ResourceFmt_R24G8_TYPELESS            :  return DXGI_FORMAT_R24G8_TYPELESS              ;
		case     GPU_ResourceFmt_D24_UNORM_S8_UINT         :  return DXGI_FORMAT_D24_UNORM_S8_UINT           ;
		case     GPU_ResourceFmt_R24_UNORM_X8_TYPELESS     :  return DXGI_FORMAT_R24_UNORM_X8_TYPELESS       ;
		case     GPU_ResourceFmt_X24_TYPELESS_G8_UINT      :  return DXGI_FORMAT_X24_TYPELESS_G8_UINT        ;
		case     GPU_ResourceFmt_R8G8_TYPELESS             :  return DXGI_FORMAT_R8G8_TYPELESS               ;
		case     GPU_ResourceFmt_R8G8_UNORM                :  return DXGI_FORMAT_R8G8_UNORM                  ;
		case     GPU_ResourceFmt_R8G8_UINT                 :  return DXGI_FORMAT_R8G8_UINT                   ;
		case     GPU_ResourceFmt_R8G8_SNORM                :  return DXGI_FORMAT_R8G8_SNORM                  ;
		case     GPU_ResourceFmt_R8G8_SINT                 :  return DXGI_FORMAT_R8G8_SINT                   ;
		case     GPU_ResourceFmt_R16_TYPELESS              :  return DXGI_FORMAT_R16_TYPELESS                ;
		case     GPU_ResourceFmt_R16_FLOAT                 :  return DXGI_FORMAT_R16_FLOAT                   ;
		case     GPU_ResourceFmt_D16_UNORM                 :  return DXGI_FORMAT_D16_UNORM                   ;
		case     GPU_ResourceFmt_R16_UNORM                 :  return DXGI_FORMAT_R16_UNORM                   ;
		case     GPU_ResourceFmt_R16_UINT                  :  return DXGI_FORMAT_R16_UINT                    ;
		case     GPU_ResourceFmt_R16_SNORM                 :  return DXGI_FORMAT_R16_SNORM                   ;
		case     GPU_ResourceFmt_R16_SINT                  :  return DXGI_FORMAT_R16_SINT                    ;
		case     GPU_ResourceFmt_R8_TYPELESS               :  return DXGI_FORMAT_R8_TYPELESS                 ;
		case     GPU_ResourceFmt_R8_UNORM                  :  return DXGI_FORMAT_R8_UNORM                    ;
		case     GPU_ResourceFmt_R8_UINT                   :  return DXGI_FORMAT_R8_UINT                     ;
		case     GPU_ResourceFmt_R8_SNORM                  :  return DXGI_FORMAT_R8_SNORM                    ;
		case     GPU_ResourceFmt_R8_SINT                   :  return DXGI_FORMAT_R8_SINT                     ;
		case     GPU_ResourceFmt_A8_UNORM                  :  return DXGI_FORMAT_A8_UNORM                    ;
	}

	unreachable();
	return DXGI_FORMAT_R8G8B8A8_UNORM;
}

//--------------------------------------------------------------------------------------
// Helper for compiling shaders with D3DCompile
// With VS 11, we could load up prebuilt .cso files instead...
//
// Memory Leak Warning:  On Intel Integrated GPU driver (i630) a memory leak occurs when
// compiling shaders.  Unknown at this time if the leak is DX11 or Intel driver.
//--------------------------------------------------------------------------------------
HRESULT TryCompileShaderFromFile(const WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut)
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
	Defer( { if (pErrorBlob) { pErrorBlob->Release(); pErrorBlob = nullptr; } } );

	// Note on compiler macros:
	//   Macros should match exactly what's being used to precompile shaders via the makefile.
	//   One possible way to do this is to have the project file write the active shader macro
	//   configuration into some larger macro that we process here and then pass to the runtime
	//   compiler.

#if defined(DX11_SHADER_COMPILER_MACROS)
	// DX11_SHADER_COMPILER_MACROS -
	//   a semicolon-delimited list of macros.  Translate it into a null-terminated macro list,
	//   because micorosoft is annoying and makes everything harder for us, the programmer.

#endif

	// Memory Leak Warning:  On Intel Integrated GPU driver (i630) a memory leak occurs when
	// compiling shaders.  Unknown at this time if the leak is DX11 affecting all GPUs, or Intel
	// driver affecting only my GPU.  --jstine

	hr = D3DCompileFromFile(szFileName, nullptr, nullptr, szEntryPoint, szShaderModel,
		dwShaderFlags, 0, ppBlobOut, &pErrorBlob);

	if (FAILED(hr)) {
		if (pErrorBlob) {
			throw_abort("%s", reinterpret_cast<const char*>(pErrorBlob->GetBufferPointer()));
		}
		elif (hr == D3D11_ERROR_FILE_NOT_FOUND || hr == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND)) {
			throw_abort("Shader file not found: %S", szFileName);
		}
		else {
			log_and_abort("D3DCompileFromFile(%S) failed with no errorBlob, hr=0x%08x", szFileName, hr);
		}
	}

	return hr;
}

ID3DBlob* CompileShaderFromFile(const WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel)
{
	// TODO: Add a cache system to avoid recompiling shaders unless the shader file has actually been modified.
	// Rationale: the leak in D3DCompileFromFile() interferes with using the Heap Graph as a leak checker.

	HRESULT hr = S_OK;
	ID3DBlob* ppBlobOut;

	hr = TryCompileShaderFromFile(szFileName, szEntryPoint, szShaderModel, &ppBlobOut);
	throw_abort_on(FAILED(hr));
	return ppBlobOut;
}

void dx11_CleanupDevice()
{
	if (!g_pd3dDevice) return;

	if (g_pImmediateContext) {
		g_pImmediateContext->ClearState();
	}

	dx11_InputLayoutCache_DisposeAll();

	for(auto& stateA : g_RasterState) {
		for(auto& stateB : stateA) {
			for(auto& stateC : stateB) {
				stateC->Release();
				stateC = nullptr;
			}
		}
	}


	if (g_pSwapChain1)			{ g_pSwapChain1			->Release();	g_pSwapChain1			= nullptr;	}
	if (g_pSwapChain)			{ g_pSwapChain			->Release();	g_pSwapChain			= nullptr;	}
	if (g_pImmediateContext1)	{ g_pImmediateContext1	->Release();	g_pImmediateContext1	= nullptr;	}
	if (g_pImmediateContext)	{ g_pImmediateContext	->Release();	g_pImmediateContext		= nullptr;	}
	if (g_pd3dDevice1)			{ g_pd3dDevice1			->Release();	g_pd3dDevice1			= nullptr;	}
	if (g_pd3dDevice)			{ g_pd3dDevice			->Release();	g_pd3dDevice			= nullptr;	}
}

__ai const	InputLayoutSlot&	GPU_InputDesc::GetSlot	(int idx)	const	{ bug_on(idx >= m_numSlots); return m_slots[idx]; }
__ai		InputLayoutSlot&	GPU_InputDesc::GetSlot	(int idx)			{ bug_on(idx >= m_numSlots); return m_slots[idx]; }

void GPU_InputDesc::_AddHash(const InputLayoutSlot& src)
{
	bug_on (!src.m_numElements, "Unexpected zero-sized src data.");

	u32 hashval = i_crc32(0, src.m_InstanceDataStepRate);
	for (int i=0; i<src.m_numElements; ++i) {
		hashval = i_crc32(hashval, src.m_Items[i].Format		);
		hashval = i_crc32(hashval, src.m_Items[i].ByteOffset	);
		static_assert(bulkof(src.m_Items[i].SemanticName) == 16, "");
		hashval = i_crc32(hashval, (u64&)src.m_Items[i].SemanticName[0]);
		hashval = i_crc32(hashval, (u64&)src.m_Items[i].SemanticName[8]);
	}

	// neat trick, serves 2 purposes.
	//  1. ensures full 64 bit hash is never zero if something meaningful has been assigned
	//     to this InputLayoutDesc.
	//  2. Allows InstanceDataStepRate to be modified later on, and also allows incrementally
	//     adding additional elements to this slot.

	m_hashval = hashval | (u64(src.m_InstanceDataStepRate+1) << 32);
}

InputLayoutSlot& InputLayoutSlot::SetInstanceStepRate(int step)
{
	m_InstanceDataStepRate = step;
	return *this;
}

InputLayoutSlot& InputLayoutSlot::Append(const InputLayoutItem& item)
{
	Append(item.SemanticName, item.Format);
	return *this;
}

InputLayoutSlot& InputLayoutSlot::Append(const InputLayoutItemEx& item)
{
	Append(item.SemanticName, item.Format, item.offset);
	return *this;
}

InputLayoutSlot& InputLayoutSlot::Append(const char* semanticName, GPU_ResourceFmt format, int offset)
{
	log_and_abort_on(m_numElements >= MaxElementsPerSlot, "Too many data elements added to InputLayoutSlot.");

	// Check for redundant semantic name ...
	for (int i=0; i<m_numElements; ++i) {
		if (strcmp(semanticName, m_Items[i].SemanticName) == 0) {
			throw_abort("Duplicate semantic name: %s", semanticName);
		}
	}

	auto& newItem = m_Items[m_numElements];
	xStrCopyT(newItem.SemanticName, semanticName);
	newItem.ByteOffset	= offset;
	newItem.Format		= format;

	m_numElements += 1;
	return *this;
}

void dx11_InitDevice()
{
	xMemZero(g_DynVertBuffers);

	HRESULT hr = S_OK;

	RECT rc;
	GetClientRect(g_hWnd, &rc);
	g_backbuffer_size_pix = {
		rc.right - rc.left,
		rc.bottom - rc.top
	};

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

	log_and_abort_on (FAILED(hr));

	//  * DX Docs recommend always having MSAA enabled.
	//  * Depth Bias options are pretty much just for shadow implementations
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
	log_and_abort_on (hr);

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

		DXGI_SWAP_CHAIN_DESC1 sd = {};
		sd.Width                = g_backbuffer_size_pix.x;
		sd.Height               = g_backbuffer_size_pix.y;
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
		DXGI_SWAP_CHAIN_DESC sd = {};
		sd.BufferCount          = 1;
		sd.BufferDesc.Width     = g_backbuffer_size_pix.x;
		sd.BufferDesc.Height    = g_backbuffer_size_pix.y;
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
	log_and_abort_on(FAILED(hr));

	// Note this tutorial doesn't handle full-screen swapchains so we block the ALT+ENTER shortcut
	dxgiFactory->MakeWindowAssociation(g_hWnd, DXGI_MWA_NO_ALT_ENTER);
	dxgiFactory->Release();

	ID3D11Texture2D* pBackBuffer = nullptr;
	hr = g_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&pBackBuffer));
	log_and_abort_on(FAILED(hr));

	auto&	rtView	= ptr_cast<ID3D11RenderTargetView*&>(g_gpu_BackBuffer.m_driverData);

	g_gpu_BackBuffer.m_driverData;
	hr = g_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &rtView);
	pBackBuffer->Release();
	log_and_abort_on(FAILED(hr));

	g_pImmediateContext->OMSetRenderTargets(1, &rtView, nullptr);

	// Setup the viewport
	D3D11_VIEWPORT vp;
	vp.Width	= (float)g_backbuffer_size_pix.x;
	vp.Height	= (float)g_backbuffer_size_pix.y;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	g_pImmediateContext->RSSetViewports(1, &vp);

	// TODO : Implement Sampler Binding API?  The most liekly variances are LINEAR/POINT sampling
	//        and texture WRAP vs. CLAMP.  Point sampling and wrapping might be useful for some
	//        types of special effects.  Most other aspects of sampling can be simulated in shaders.
	//        (note: 2D game engine should not care about anisotropic or mip-mapped effects)

	pragma_todo("Implement Sampler Binding API.")

	D3D11_SAMPLER_DESC samplerDesc = {};

//	samplerDesc.Filter			= D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.Filter			= D3D11_FILTER_MIN_MAG_MIP_POINT;
	samplerDesc.AddressU		= D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressV		= D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressW		= D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.ComparisonFunc	= D3D11_COMPARISON_NEVER;
	samplerDesc.MinLOD			= 0;
	samplerDesc.MaxLOD			= D3D11_FLOAT32_MAX;

	// Create the sampler
	hr = g_pd3dDevice->CreateSamplerState( &samplerDesc, &m_pTextureSampler);
	bug_on(FAILED(hr));

	XMVECTOR Eye	= XMVectorSet( 0.0f, 1.0f, -5.0f, 0.0f );
	XMVECTOR At		= XMVectorSet( 0.0f, 1.0f, 0.0f, 0.0f );
	XMVECTOR Up		= XMVectorSet( 0.0f, 1.0f, 0.0f, 0.0f );

	g_World			= XMMatrixIdentity();
	g_View			= XMMatrixLookAtLH( Eye, At, Up );
	g_Projection	= XMMatrixPerspectiveFovLH( XM_PIDIV2, float(g_backbuffer_size_pix.x) / float(g_backbuffer_size_pix.y), 0.01f, 100.0f );

	log_and_abort_on(FAILED(hr));
}

#include <unordered_map>

struct dx11_ShaderInfo
{
	ID3DBlob*		blob	= nullptr;
	u32				hash	= 0;

	void Dispose() {
		blob->Release();
		blob = nullptr;
		hash = 0;
	}
};

struct InputLayoutCacheItem {
	ID3D11InputLayout*		dx	= nullptr;			// dx specific driver handle
	u32			 			inputDescHash;			// hash index into s_dx11_InputDescCache

	// * any number of shaders might reference this InputLayout.
	// * Destroying InputLayouts doesn't cause failure, just reduces performance.
	// * Memory usage from InputLayouts is probably small
	// * Barring open-world things, we have set places where input layouts can be wiped clean
	// * Could add a feature to mark certain input layouts as "persistent"
	// * To determine important IAs, add command to wipe all IAs and then log out all IAs generated
	//   on the next frame(s).

};

using InputLayoutCache_t	= std::unordered_map<u32,					InputLayoutCacheItem,	FunctHashIdentity>;
using InputDescCache_t		= std::unordered_map<GPU_InputDescHash_t,	GPU_InputDesc,			FunctHashIdentity>;

static u32		s_CurrentInputDescHash = 0;
static bool		s_NeedsPreDrawPrep = 0;
static const GPU_InputDesc*		s_CurrentInputDesc	= nullptr;
static const GPU_ShaderVS*		s_CurrentShaderVS	= nullptr;
static const GPU_ShaderFS*		s_CurrentShaderFS	= nullptr;

static InputLayoutCache_t		s_dx11_InputLayoutCache;
//static InputDescCache_t			s_dx11_InputDescCache;

void dx11_InputLayoutCache_DisposeAll()
{
	for (auto& item : s_dx11_InputLayoutCache) {
		if (item.second.dx) { item.second.dx->Release(); item.second.dx = nullptr; }
	}

	s_dx11_InputLayoutCache.clear();
}

void dx11_SetInputLayout(const GPU_InputDesc& layout)
{
	// layout will be resolved against bound shaders when the draw command is initiated.
	// If the layout is fresh (unrecognized hash) then it will be added to the internal layout cache.

	if (!s_CurrentInputDesc || (s_CurrentInputDesc->GetHash() != layout.GetHash())) {
		s_NeedsPreDrawPrep		= 1;
		s_CurrentInputDesc		= &layout;

		//if (s_dx11_InputDescCache.find(s_CurrentInputDescHash) == s_dx11_InputDescCache.end()) {
		//	log_perf( "Adding new InputDescription, hash=0x%08x total=%d", s_CurrentInputDescHash, s_dx11_InputDescCache.size()+1);
		//	s_dx11_InputDescCache.insert( { s_CurrentInputDescHash, layout } );
		//}
	}
}

ID3D11InputLayout* do_prep_inputLayout()
{
	throw_abort_on(!s_CurrentInputDesc,		"No input layout has been bound to the pipeline.");
	throw_abort_on(!s_CurrentShaderVS,		"No vertex shader has been bound to the pipeline.");
	pragma_todo("Use ID3D11ShaderReflection::GetInputParameterDesc() to determine shader-to-inputlayout association.");


	const auto& shader	= ptr_cast<ID3D11VertexShader* const &>	(s_CurrentShaderVS->m_driverBinary);
	const auto& info	= ptr_cast<dx11_ShaderInfo* const &>	(s_CurrentShaderVS->m_driverBlob);

	u32 fullhash_vs = i_crc32(s_CurrentInputDescHash, info->hash);

	if (1) {
		auto& it = s_dx11_InputLayoutCache.find(fullhash_vs);
		if (it != s_dx11_InputLayoutCache.end()) {
			return it->second.dx;
		}
	}

	// Generate a DX11 input layout from our custom input layout system.
	// Hard limited to 16 since the dx11 default of 32 may not be widely supported on mobile-ish
	// platforms, and likely we would never need such extreme shaders for our apps... -- jstine

	D3D11_INPUT_ELEMENT_DESC dx_layout[16] = {};

	//const auto  it			= s_dx11_InputDescCache.find(s_CurrentInputDescHash);
	//const auto& inputDesc	= it->second;

	const auto& inputDesc	= *s_CurrentInputDesc;

	int dxidx = 0;
	for (int i=0; i<inputDesc.GetSlotCount(); ++i) {
		const auto& slot = inputDesc.GetSlot(i);

		for (int t=0; t<slot.m_numElements; ++t) {
					auto& dxlay	= dx_layout[dxidx];
			const	auto& item	= slot.m_Items[t];

			dxlay.InstanceDataStepRate	= slot.m_InstanceDataStepRate;

			// Semantic Index should be the last digit of the SemanticName
			// If no digit, assume it must be ZERO.
			// (why doesn't DX11 do this for us? I know, probably legacy from DX9)

			int semanticLen = strlen(item.SemanticName);
			char c = item.SemanticName[semanticLen-1];
			int semidx = 0;

			if (isdigit(c)) {
				semidx = c - '0';
			}

			dxlay.Format			= get_DXGI_Format(item.Format);
			dxlay.AlignedByteOffset = (item.ByteOffset<0)			? D3D11_APPEND_ALIGNED_ELEMENT	: item.ByteOffset;
			dxlay.InputSlotClass	= (slot.m_InstanceDataStepRate) ? D3D11_INPUT_PER_INSTANCE_DATA : D3D11_INPUT_PER_VERTEX_DATA;
			dxlay.SemanticName		= item.SemanticName;
			dxlay.SemanticIndex		= semidx;
			dxlay.InputSlot			= i;
			dxidx += 1;
		}
	}

	InputLayoutCacheItem newCacheItem;
	newCacheItem.inputDescHash = s_CurrentInputDesc->GetHash();

	pragma_todo("Add user-defined long-name description to InputDesc for logging and debugging.");
	log_perf( "Adding new InputLayout to cache, hash=0x%08x count=%d", fullhash_vs, s_dx11_InputLayoutCache.size() );

	HRESULT hr;
	hr = g_pd3dDevice->CreateInputLayout(
		dx_layout, dxidx,
		info->blob->GetBufferPointer(),
		info->blob->GetBufferSize(),
		&newCacheItem.dx
	);

	s_dx11_InputLayoutCache.insert( { fullhash_vs, newCacheItem } );
	return newCacheItem.dx;
}

void dx11_PreDrawPrep()
{
	if (!s_NeedsPreDrawPrep) return;
	throw_abort_on(!s_CurrentShaderVS, "No VS shader is bound to the draw pipeline.");
	throw_abort_on(!s_CurrentShaderFS, "No FS shader is bound to the draw pipeline.");

	auto&	shaderVS	= ptr_cast<ID3D11VertexShader*	const &>(s_CurrentShaderVS->m_driverBinary);
	auto&	shaderFS	= ptr_cast<ID3D11PixelShader*	const &>(s_CurrentShaderFS->m_driverBinary);
	bug_on_qa(!shaderVS, "VS shader resource has been deinitialized since being bound.");
	bug_on_qa(!shaderFS, "FS shader resource has been deinitialized since being bound.");

	g_pImmediateContext->IASetInputLayout(do_prep_inputLayout());
	g_pImmediateContext->VSSetShader(shaderVS, nullptr, 0);
	g_pImmediateContext->PSSetShader(shaderFS, nullptr, 0);
	s_NeedsPreDrawPrep = 0;
}

template< typename T >
T* xReallocObj(T* &ptr)
{
	pragma_todo( "Check that xRealloc is smart and re-uses existing pointer data if sizes match..." );
	ptr->~T();
	ptr = new ((T*)xRealloc(ptr, sizeof(*ptr))) T();
	return ptr;
}

template< typename T >
T* xReallocT(T* &ptr)
{
	static_assert(!std::has_virtual_destructor<T>::value, "Non-trivial type has meaningful destructor.  Use xReallocObj<T> instead.")
	//static_assert(!std::is_trivially_copyable<T>::value, "Non-trivial type has meaningful destructor.  Use xReallocObj<T> instead.")
	ptr = xRealloc(ptr, sizeof(*ptr));
	return ptr;
}

bool dx11_TryLoadShaderVS(GPU_ShaderVS& dest, const xString& srcfile, const char* entryPointFn)
{
	HRESULT hr;

	auto& shader	= ptr_cast<ID3D11VertexShader* &>	(dest.m_driverBinary);
	auto& info		= ptr_cast<dx11_ShaderInfo* &>		(dest.m_driverBlob);

	if (shader) { shader->Release(); shader	= nullptr; }
	if (info)	{ info	->Dispose(); }

	xReallocObj(info);

	bug_on( !entryPointFn || !entryPointFn[0] );
	info->blob = CompileShaderFromFile(toUTF16(srcfile).wc_str(), entryPointFn, "vs_4_0");
	if (!info->blob) return false;

	const u64* ptr64	= (u64*)info->blob->GetBufferPointer();
	int size64	= info->blob->GetBufferSize() / 8;
	int sizeRem	= info->blob->GetBufferSize() & 7;

	u32 hash = 0;
	for (int i=0; i<size64; ++i, ++ptr64) {
		hash = i_crc32(hash, ptr64[0]);
	}

	const u8* ptr8	= (u8*)ptr64;
	for (int i=0; i<sizeRem; ++i, ++ptr8) {
		hash = i_crc32(hash, ptr8[0]);
	}
	info->hash = hash;

	hr = g_pd3dDevice->CreateVertexShader(info->blob->GetBufferPointer(), info->blob->GetBufferSize(), nullptr, &shader);
	log_and_abort_on(FAILED(hr));

	return true;
}

bool dx11_TryLoadShaderFS(GPU_ShaderFS& dest, const xString& srcfile, const char* entryPointFn)
{
	HRESULT hr;

	auto& shader	= ptr_cast<ID3D11PixelShader* &>	(dest.m_driverBinary);
	auto& info		= ptr_cast<dx11_ShaderInfo* &>		(dest.m_driverBlob);

	if (shader) { shader->Release(); shader = nullptr; }
	if (info)	{ info	->Dispose(); }

	xReallocObj(info);

	bug_on( !entryPointFn || !entryPointFn[0] );
	info->blob = CompileShaderFromFile(toUTF16(srcfile).wc_str(), entryPointFn, "ps_4_0");
	if (!info->blob) return false;

	hr = g_pd3dDevice->CreatePixelShader(info->blob->GetBufferPointer(), info->blob->GetBufferSize(), nullptr, &shader);
	log_and_abort_on(FAILED(hr));

	info->blob->Release();
	info->blob = nullptr;
	return true;
}

void dx11_LoadShaderVS(GPU_ShaderVS& dest, const xString& srcfile, const char* entryPointFn)
{
	auto result = dx11_TryLoadShaderVS(dest, srcfile, entryPointFn);
	bug_on_qa(!result, "Errors during shader compiler and no error handler is registered.");
}

void dx11_LoadShaderFS(GPU_ShaderFS& dest, const xString& srcfile, const char* entryPointFn)
{
	auto result = dx11_TryLoadShaderFS(dest, srcfile, entryPointFn);
	bug_on_qa(!result, "Errors during shader compiler and no error handler is registered.");
}

void dx11_BindShaderVS(const GPU_ShaderVS& vs)
{
	bug_on_qa(!vs.m_driverBinary, "Uninitialized VS shader resource.");
	if (!s_CurrentShaderVS || (s_CurrentShaderVS != &vs)) {
		s_CurrentShaderVS = &vs;
		s_NeedsPreDrawPrep = 1;
	}
}

void dx11_BindShaderFS(const GPU_ShaderFS& fs)
{
	bug_on_qa(!fs.m_driverBinary, "Uninitialized FS shader resource.");
	if (!s_CurrentShaderFS || (s_CurrentShaderFS != &fs)) {
		s_CurrentShaderFS = &fs;
		s_NeedsPreDrawPrep = 1;
	}
}

void dx11_SetVertexBuffer(const GPU_DynVsBuffer& src, int shaderSlot, int _stride, int _offset)
{
	uint stride = _stride;
	uint offset = _offset;

	bug_on(!src.IsValid());
	if (!src.IsValid()) return;

	auto& buffer = g_DynVertBuffers[g_curBufferIdx][src.m_buffer_idx];
	bug_on_qa(buffer.m_type != DynBuffer_Vertex, "DynamicVertexBuffer expected '%s' but got '%s'",
		enumToString(DynBuffer_Vertex),
		enumToString(buffer.m_type)
	);
	g_pImmediateContext->IASetVertexBuffers(shaderSlot, 1, &buffer.m_dx11_buffer, &stride, &offset);
}

void dx11_SetVertexBuffer( const GPU_VertexBuffer& vbuffer, int shaderSlot, int _stride, int _offset)
{
	uint stride = _stride;
	uint offset = _offset;

	g_pImmediateContext->IASetVertexBuffers(shaderSlot, 1, (ID3D11Buffer**)&vbuffer.m_driverData, &stride, &offset);
}

void dx11_SetIndexBuffer(const GPU_IndexBuffer& indexBuffer, int bitsPerIndex, int offset)
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

void dx11_Draw(int indexCount, int startVertLoc)
{
	dx11_PreDrawPrep();
	g_pImmediateContext->Draw(indexCount, startVertLoc);
}

void dx11_DrawIndexed(int indexCount, int startIndexLoc, int baseVertLoc)
{
	dx11_PreDrawPrep();
	g_pImmediateContext->DrawIndexed(indexCount, startIndexLoc, baseVertLoc);
}

void dx11_DrawInstanced(int vertsPerInstance, int instanceCount, int startVertLoc, int startInstanceLoc)
{
	dx11_PreDrawPrep();
	g_pImmediateContext->DrawInstanced(vertsPerInstance, instanceCount, startVertLoc, startInstanceLoc);
}

void dx11_DrawIndexedInstanced(int indexesPerInstance, int instanceCount, int startIndex, int baseVertex, int startInstance)
{
	dx11_PreDrawPrep();
	g_pImmediateContext->DrawIndexedInstanced(indexesPerInstance, instanceCount, startIndex, baseVertex, startInstance);
}

void dx11_UploadDynamicBufferData(const GPU_DynVsBuffer& src, const void* srcData, int sizeInBytes)
{
	// Trying to decide between assert or silent ignore if the buffer is not initialized...
	// Or we could have a "dx11_TryUploadDynamicBufferData" too!

	bug_on(!src.IsValid());
	if (!src.IsValid()) return;

	D3D11_MAPPED_SUBRESOURCE mappedResource = {};

	auto&   simple      = g_DynVertBuffers[g_curBufferIdx][src.m_buffer_idx];

	bug_on_qa(simple.m_type == DynBuffer_Free);
	g_pImmediateContext->Map(simple.m_dx11_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	xMemCopy(mappedResource.pData, srcData, sizeInBytes);
	g_pImmediateContext->Unmap(simple.m_dx11_buffer, 0);
}

void dx11_CreateDynamicVertexBuffer(GPU_DynVsBuffer& dest, int bufferSizeInBytes)
{
	// TODO : Improve search algo efficiency...
	int bufferIdx = dest.m_buffer_idx;

	if (dest.m_buffer_idx < 0) {
		for(int i=0; i<bulkof(g_DynVertBuffers[0]); ++i) {
			if (g_DynVertBuffers[0][i].m_type == DynBuffer_Free) {
				bufferIdx = i;
				break;
			}
		}
	}

	log_and_abort_on(bufferIdx < 0, "Ran out of Dynamic buffer handles");

	for (int i=0; i<BackBufferCount; ++i) {
		D3D11_BUFFER_DESC bd = {};
		bd.Usage			= D3D11_USAGE_DYNAMIC;
		bd.ByteWidth		= bufferSizeInBytes;
		bd.BindFlags		= D3D11_BIND_VERTEX_BUFFER;
		bd.CPUAccessFlags	= D3D11_CPU_ACCESS_WRITE;

		auto& buffer = g_DynVertBuffers[i][bufferIdx];
		if (buffer.m_dx11_buffer) {
			buffer.m_dx11_buffer->Release();
			buffer.m_dx11_buffer = nullptr;
		}
		auto hr = g_pd3dDevice->CreateBuffer(&bd, nullptr, &buffer.m_dx11_buffer);
		bug_on (FAILED(hr));
		buffer.m_type = DynBuffer_Vertex;
	}

	dest.m_buffer_idx = bufferIdx;
}

void dx11_CreateStaticMesh(GPU_VertexBuffer& dest, void* vertexData, int itemSizeInBytes, int vertexCount)
{
	auto&	buffer	= ptr_cast<ID3D11Buffer*&>(dest.m_driverData );
	if (dest.m_driverData) { buffer	->Release(); buffer	= nullptr; }

	D3D11_BUFFER_DESC bd = {};

	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = itemSizeInBytes * vertexCount;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;
	D3D11_SUBRESOURCE_DATA InitData = {};
	InitData.pSysMem = vertexData;

	GPU_VertexBuffer result;
	auto hr = g_pd3dDevice->CreateBuffer( &bd, &InitData, &buffer );
	bug_on(FAILED(hr));
}

void dx11_CreateIndexBuffer(GPU_IndexBuffer& dest, void* indexBuffer, int bufferSize)
{
	auto&	buffer	= ptr_cast<ID3D11Buffer*&>(dest.m_driverData );
	if (dest.m_driverData) { buffer	->Release(); buffer	= nullptr; }

	D3D11_SUBRESOURCE_DATA	InitData	= {};
	D3D11_BUFFER_DESC		bd			= {};

	bd.Usage			= D3D11_USAGE_DEFAULT;
	bd.ByteWidth		= bufferSize;
	bd.BindFlags		= D3D11_BIND_INDEX_BUFFER;
	bd.CPUAccessFlags	= 0;
	InitData.pSysMem	= indexBuffer;

	auto hr = g_pd3dDevice->CreateBuffer( &bd, &InitData, &buffer );
	bug_on (FAILED(hr));
}

void dx11_CreateConstantBuffer(GPU_ConstantBuffer& dest, int bufferSize)
{
	auto&	buffer	= ptr_cast<ID3D11Buffer*&>(dest.m_driverData);
	if (dest.m_driverData) { buffer	->Release(); buffer	= nullptr; }

    D3D11_BUFFER_DESC bd = {};
	bd.Usage			= D3D11_USAGE_DEFAULT;
	bd.ByteWidth		= (bufferSize + 15) & ~15;
	bd.BindFlags		= D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags	= 0;
    auto hr = g_pd3dDevice->CreateBuffer( &bd, nullptr, &buffer );
    bug_on (FAILED(hr));
}

void dx11_UpdateConstantBuffer(const GPU_ConstantBuffer& buffer, const void* data)
{
	auto&	drvbuf	= ptr_cast<ID3D11Buffer* const &>(buffer.m_driverData);
	bug_on(!drvbuf, "Uninitialized ConstantBuffer resource");
	g_pImmediateContext->UpdateSubresource(drvbuf, 0, nullptr, data, 0, 0 );

	// Implementation warning:  Yes, nvidia suggests using map(DISCARD)/unmap() instead of UpdateSubresource.
	// This is not to be implemented blindly.
	//  1. using map/memcpy/unmap within this function alone may not help.  The point is to avoid the memcpy()
	//     not to avoid the call to UpdateSubresource (which calls memcpy internally).  Thus, to use map/unmap
	//     correctly, the new data must be calculated and written directly to GPU, bypassing *any* heap-allocated
	//     staging area.  Only then is memcpy avoided.
	//
	//  2. using map/calc/write/unmap without our own double-buffering of the resource is risky.  Not all GPU
	//     drivers can be expected to perform renaming operations on MAP_DISCARD.  If buffer renaming is not
	//     supported by the driver, then attempts to update the buffer will lead to CPU/GPU stalls.   In other
	//     words:  good perf for nvidia users, maybe tragically bad perf for people running intel drivers that
	//     came with their Windows OEM.
	//
	// On the other hand, there's a good chance that our implementation of xMemCopy is more efficient than the
	// driver's implementation.  The driver likely uses some overblown memcpy() that does 8 size checks before
	// moving a single byte of data, and is terribly slow for small-data constant updates.  Ours uses movsd,
	// and could be optimized further by providing a template version of this function that knows constant size
	// of the input data.
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
	g_pImmediateContext->PSSetSamplers( 0, 1, &m_pTextureSampler );
}

void dx11_BindShaderResource(const GPU_ShaderResource& res, int startSlot)
{
	auto&	resourceView	= ptr_cast<ID3D11ShaderResourceView* const&>(res.m_driverData_view);
	g_pImmediateContext->VSSetShaderResources( startSlot, 1, &resourceView );
	g_pImmediateContext->PSSetShaderResources( startSlot, 1, &resourceView );
}

void dx11_BindConstantBuffer(const GPU_ConstantBuffer& buffer, int startSlot)
{
	auto&	drvbuf			= ptr_cast<ID3D11Buffer* const &>(buffer.m_driverData);
	g_pImmediateContext->VSSetConstantBuffers(startSlot, 1, &drvbuf);
	g_pImmediateContext->PSSetConstantBuffers(startSlot, 1, &drvbuf);
}


void dx11_CreateTexture2D(GPU_TextureResource2D& dest, const void* src_bitmap_data, int width, int height, GPU_ResourceFmt format)
{
	HRESULT hr;

	auto&	texture		= ptr_cast<ID3D11Texture2D*&>			(dest.m_driverData_tex );
	auto&	textureView	= ptr_cast<ID3D11ShaderResourceView*&>	(dest.m_driverData_view);

	if (dest.m_driverData_view) { textureView	->Release(); textureView	= nullptr; }
	if (dest.m_driverData_tex)	{ texture		->Release(); texture		= nullptr; }

	// See if format is supported for auto-gen mipmaps (varies by feature level)
	// (Must have context and shader-view to auto generate mipmaps)

	DXGI_FORMAT dxfmt = get_DXGI_Format(format);

	bool autogen_mipmaps = false;
	UINT fmtSupport = 0;
	hr = g_pd3dDevice->CheckFormatSupport(dxfmt, &fmtSupport);
	if (SUCCEEDED(hr) && (fmtSupport & D3D11_FORMAT_SUPPORT_MIP_AUTOGEN))
	{
		autogen_mipmaps = true;
	}

	// Create texture
	D3D11_TEXTURE2D_DESC desc = {};

	desc.Width				= width;
	desc.Height				= height;
	desc.MipLevels			= (autogen_mipmaps) ? 0 : 1;
	desc.ArraySize			= 1;
	desc.Format				= dxfmt;
	desc.SampleDesc.Count	= 1;
	desc.SampleDesc.Quality = 0;
	desc.Usage				= D3D11_USAGE_DEFAULT;
	desc.CPUAccessFlags		= 0;

	desc.BindFlags			= D3D11_BIND_SHADER_RESOURCE;
	desc.MiscFlags			= 0;

	if (autogen_mipmaps)
	{
		desc.BindFlags	   |= D3D11_BIND_RENDER_TARGET;
		desc.MiscFlags	   |= D3D11_RESOURCE_MISC_GENERATE_MIPS;
	}

	int bytespp = 4;
	if (format == GPU_ResourceFmt_R32G32_UINT) {
		bytespp = 8;
	}

	if (format == GPU_ResourceFmt_R32G32_FLOAT) {
		bytespp = 8;
	}

	int rowPitch = ((width * bytespp));

	D3D11_SUBRESOURCE_DATA initData;
	initData.pSysMem			= src_bitmap_data;
	initData.SysMemPitch		= rowPitch;
	initData.SysMemSlicePitch	= rowPitch * height;

	hr = g_pd3dDevice->CreateTexture2D(&desc, (autogen_mipmaps) ? nullptr : &initData, &texture);
	log_and_abort_on(FAILED(hr) || !texture);

	// textureView bits

	D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc = {};
	SRVDesc.Format				= desc.Format;
	SRVDesc.ViewDimension		= D3D11_SRV_DIMENSION_TEXTURE2D;
	SRVDesc.Texture2D.MipLevels = autogen_mipmaps ? -1 : 1;

	hr = g_pd3dDevice->CreateShaderResourceView(texture, &SRVDesc, &textureView);
	log_and_abort_on (FAILED(hr));

	if (autogen_mipmaps)
	{

#if defined(_XBOX_ONE) && defined(_TITLE)
		// reference code left in from when I copied this mess from DirectXTK.
		// Has not been updated or tested.  --jstine

		ID3D11Texture2D *pStaging = nullptr;
		CD3D11_TEXTURE2D_DESC stagingDesc(format, twidth, theight, 1, 1, 0, D3D11_USAGE_STAGING, D3D11_CPU_ACCESS_READ, 1, 0, 0);
		initData.pSysMem = temp.get();
		initData.SysMemPitch = static_cast<UINT>(rowPitch);
		initData.SysMemSlicePitch = static_cast<UINT>(imageSize);

		hr = d3dDevice->CreateTexture2D(&stagingDesc, &initData, &pStaging);
		if (SUCCEEDED(hr))
		{
			d3dContext->CopySubresourceRegion(tex, 0, 0, 0, 0, pStaging, 0, nullptr);

			UINT64 copyFence = d3dContextX->InsertFence(0);
			while (d3dDeviceX->IsFencePending(copyFence)) { SwitchToThread(); }
			pStaging->Release();
		}
#else
		g_pImmediateContext->UpdateSubresource(texture, 0, nullptr, src_bitmap_data, rowPitch, rowPitch * height);
#endif
		g_pImmediateContext->GenerateMips(textureView);
	}
}

void dx11_ClearRenderTarget(const GPU_RenderTarget& target, const float4& color)
{
	g_pImmediateContext->ClearRenderTargetView((ID3D11RenderTargetView*)target.m_driverData, color.f);
}
