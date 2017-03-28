
#define TARGET_DEBUG		1

#include "x-types.h"
#include "x-stl.h"
#include "x-assertion.h"
#include "x-string.h"
#include "x-thread.h"

#include "Bezier2D.h"

#include <chrono>

#include <windows.h>
#include <d3d11_1.h>
#include <d3dcompiler.h>
#include <directxmath.h>
#include <DirectXColors.h>

#include "Bezier2d.inl"

using namespace DirectX;

static const int BackBufferCount = 3;
static const int const_zval = 0.1f;

struct DynamicVertexBufferSet {

	// Vertex lists will need to be split according to their layout - namely their stride.
	// eg, if some have texture UV data while others do not.
	// It makes the most sense if the buffers are named after the vertex data types that they contain.
	// Or more generically they can be named according to the buffer stride, since the underlying data
	// is not important.

	// For now just have the Simple buffer...

	ID3D11Buffer*      Simple       = nullptr;
};


// * Vertex Buffers are Mostly Dynamic.
// * Use rotating buffers to avoid blocking on prev frame in order to setup new frame.
// * Index Buffers use series of "Default Layouts" which can be packed into a single buffer.



HINSTANCE               g_hInst					= nullptr;
HWND                    g_hWnd					= nullptr;
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


DynamicVertexBufferSet  g_DynVertBuffers[BackBufferCount];
//DynamicIndexBufferSet


ID3D11Buffer*           g_pIndexBuffer			= nullptr;

ID3D11Buffer*           g_pConstantBuffer		= nullptr;

XMMATRIX                g_World;
XMMATRIX                g_View;
XMMATRIX                g_Projection;

std::chrono::steady_clock::time_point s_HostLogTimer;



// --------------------------------------------------------------------------------------
bool xIsDebuggerAttached()
{
	return !! ::IsDebuggerPresent();
}

// --------------------------------------------------------------------------------------
void xOutputVerboseString(const char* str)
{
#if MSW_ENABLE_DEBUG_OUTPUT
	if( 1 /*::IsDebuggerPresent()*/ )
		OutputDebugStringA(str);
	else
	{
		fputs(str, stdout);
		return;
	}
#endif

#ifdef _CONSOLE
	fputs(str, stdout);			// avoids puts() spurious newline.
#endif
}

void xOutputDebugString(const char* str)
{
#if MSW_ENABLE_DEBUG_OUTPUT
	if( 1 /*::IsDebuggerPresent()*/ )
		OutputDebugStringA(str);
	else
	{
		fputs(str, stdout);
		return;
	}
#endif

#ifdef _CONSOLE
	fputs(str, stdout);			// avoids puts() spurious newline.
#endif
}

// --------------------------------------------------------------------------------------
static void _debugBreakContext( xString& context, const char* filepos, const char* funcname, const char* cond )
{
	const char* threadname = thread_getname();

	context.Format(
		"Condition  :  %s\n"
		"Function   :  %s\n"
		"Thread     :  %s\n",

		cond ? cond : "(none)",
		funcname,
		threadname ? threadname : "(unknown)"
	);
}

static void _flush_all_that_filesystem_jazz()
{
	fflush(nullptr);
}

static __ai assert_t DoAssertionDialog( const xString& title, const xString& content )
{
//	AtomicInc( s_open_assertion_counter );

	auto result = MessageBox(nullptr,
		toUTF16(content).wc_str(), toUTF16(title).wc_str(), MB_ABORTRETRYIGNORE
	);

//	AtomicDec( s_open_assertion_counter );

	if (result == IDRETRY)
	{
		xPrintLn_loud( "*** DEBUG BREAK REQUESTED ***" );
		return assert_break;
	}
	else if (result == IDABORT)
	{
		xPrintLn_loud( "*** TERMINATION REQUESTED ***" );
		return assert_terminate_app;
	}
	else if (result == IDIGNORE)
	{
		xPrintLn_loud( "*** all assertions of this type ignored! ***" );
		return assert_ignore_all;
	}
	return assert_none;
}

__eai assert_t Host_AssertionDialog( const xString& title, const xString& message, const xString& context )
{
//	if (Host_IsUsingCLI())		return assert_break;
	if (IsDebuggerPresent())	return assert_break;

	return DoAssertionDialog( title, message + "\n\nAssertionContext:\n" + context );
}

// --------------------------------------------------------------------------------------
assert_t xDebugBreak_v( DbgBreakType breakType, const char* filepos, const char* funcname, const char* cond, const char* fmt, va_list list )
{
	static __threadlocal int s_recursion_guard = 0;

	xScopedIncremental	guard( s_recursion_guard );

	if (!guard.TryEnter())
	{
		__debugbreak();
		exit(66);
	}

	xString context;
	_debugBreakContext( context, filepos, funcname, cond );

	xString message;
	if (fmt) {
		message.FormatV(fmt, list);
	}

	const char* title = "Application Error";

	switch (breakType) {
		case DbgBreakType_Assert:	title = "Assertion Failure";		break;
		case DbgBreakType_Abort:	title = "Application Error";		break;

		default:
			xPrintLn_loud(xFmtStr("Unknown DbgBreakType Type = %d will be treated as abort (Application Error)", breakType));
			title = "Application Error";
		break;
	}

	xPrintLn_loud( xFmtStr("%s: *** ASSERTION FAILURE ***\n%s\n\nContext:\n%s", filepos, message.c_str(), context.c_str()) );
	_flush_all_that_filesystem_jazz();

	assert_t breakit = assert_none;

	breakit = Host_AssertionDialog( title, message, context );

	if (breakit == assert_terminate_app)
	{
		// this will no doubt fail miserably.  Need a custom exit() function that calls various
		// thread shutdown destructors in the appropriate order.

		exit(-1);
	}

	return breakit;
}

// --------------------------------------------------------------------------------------
assert_t xDebugBreak( DbgBreakType breakType, const char* filepos, const char* funcname, const char* cond, const char* fmt, ... )
{
	va_list list;
	va_start(list, fmt);
	assert_t result = xDebugBreak_v( breakType, filepos, funcname, cond, fmt, list );
	va_end(list);

	return result;
}


// s_mylog_MaxFileSize
//  Above this length, the current logfile is closed and a new one is opened for continue log
//  generation.  Some editors can handle larger files (1-2gb is commonl allowable) but they
//  tend to handle those sizes poorly -- especially when doing diffs.  So splitting them up
//  usually is a low-hassle way to speed up viewing/diffing. --jstine
//
const			s64				s_mylog_MaxFileSize	= _256mb;
const			s64				s_spam_maxSize		= _4gb * 4;		// a generous 16gb!

static xMutex	s_mtx_unilogger;
static FILE*	s_myLog				= nullptr;
s64				s_myLog_Written		= 0;

static void _openNewLogFile()
{
	todo("oops!  logfile not implemented.");
}

static void advanceMyLog( int numChars )
{
	s_myLog_Written += numChars;
	if (s_myLog_Written > s_mylog_MaxFileSize)
	{
		_openNewLogFile();
	}
}

// --------------------------------------------------------------------------------------
static void vlog_append_host_clock(xString& dest)
{
	xScopedMutex lock(s_mtx_unilogger);

	auto newtime = std::chrono::steady_clock::now();

	lock.Unlock();

	typedef std::chrono::duration<double> seconds_d;
	auto secs = std::chrono::duration_cast<seconds_d>(newtime - s_HostLogTimer);
	dest.AppendFmt("[%8.03fsec] ", secs);
}

// --------------------------------------------------------------------------------------
template<bool isImportant>
void xPrintLn_impl(const xString& msg)
{
	xScopedMutex lock(s_mtx_unilogger);

	//if(vlog_check_history(LogType_Verbose, NULL, msg))
	//	return;

	xString buffer = "        ";
	vlog_append_host_clock(buffer);
	buffer += msg;
	buffer += "\n";

	if (isImportant)		{ xOutputVerboseString(buffer); }
	else					{ xOutputDebugString  (buffer); }
	if (s_myLog)			{ fputs(buffer, s_myLog);		}
}


void xPrintLn		(const xString& msg) { xPrintLn_impl<false>(msg); }
void xPrintLn_loud	(const xString& msg) { xPrintLn_impl<true >(msg); }

// --------------------------------------------------------------------------------------
// Host logs work almost the same as generic verbose logs, except they disregard virtual
// ps2 cpu context info and always print host-centric time/context info instead.
//
void _host_log(uint flags, const char* moduleName, const char* fmt, ...)
{
	if (!fmt || !fmt[0])
	{
		// just treat it as a newline, no prefixing or other mess.
		// (sometimes used to generate visual separation in the console output)

		xScopedMutex lock(s_mtx_unilogger);
		xOutputDebugString("\n");
		if (s_myLog) fputs("\n", s_myLog);
		return;
	}

	xString buffer;
	if (moduleName && moduleName[0])
		buffer.Format("%-8s", moduleName);
	else
		buffer = "        ";

	vlog_append_host_clock(buffer);

	if (fmt && fmt[0])
	{
		va_list list;
		va_start(list, fmt);
		buffer.AppendFmtV(fmt, list);
		buffer += "\n";
		va_end(list);
	}

	xScopedMutex lock(s_mtx_unilogger);

	if (flags & xLogFlag_Important) { xOutputVerboseString(buffer); }
	else							{ xOutputDebugString  (buffer); }

	if (s_myLog)
	{
		fputs(buffer, s_myLog);
		//fflush(s_myLog);		// fflush(nullptr) performend on sigsegv handler.
		advanceMyLog(buffer.GetLength() + 10);
	}

	//spamAbortCheck(buffer.GetLength() + 10);
}

//--------------------------------------------------------------------------------------
// WINDOWS BOILERPLATE
//--------------------------------------------------------------------------------------
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC hdc;

	switch (message)
	{
		case WM_PAINT:
			hdc = BeginPaint(hWnd, &ps);
			EndPaint(hWnd, &ps);
			break;

		case WM_DESTROY:
			PostQuitMessage(0);
			break;

			// Note that this tutorial does not handle resizing (WM_SIZE) requests,
			// so we created the window without the resize border.

		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
	}

	return 0;
}

HRESULT InitWindow(HINSTANCE hInstance, int nCmdShow)
{
	// Register class
	WNDCLASSEX wcex;
	wcex.cbSize			= sizeof(WNDCLASSEX);
	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(NULL, IDI_APPLICATION); //LoadIcon(hInstance, (LPCTSTR)IDI_TUTORIAL1);
	wcex.hIconSm		= LoadIcon(NULL, IDI_APPLICATION); //LoadIcon(wcex.hInstance, (LPCTSTR)IDI_TUTORIAL1);
	wcex.hCursor		= LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName	= nullptr;
	wcex.lpszClassName	= L"GameClass";
	if (!RegisterClassEx(&wcex))
		return E_FAIL;

	// Create window
	g_hInst = hInstance;
	RECT rc = { 0, 0, 800, 600 };
	AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);
	g_hWnd = CreateWindow(L"GameClass", L"Playable Game Thing",
		WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
		CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, nullptr, nullptr, hInstance,
		nullptr);
	if (!g_hWnd)
		return E_FAIL;

	ShowWindow(g_hWnd, nCmdShow);

	return S_OK;
}

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

	// set default render state to msaa enabled
	D3D11_RASTERIZER_DESC drd = {
		D3D11_FILL_SOLID, //D3D11_FILL_MODE FillMode;
		D3D11_CULL_NONE,//D3D11_CULL_MODE CullMode;
		FALSE, //BOOL FrontCounterClockwise;
		0, //INT DepthBias;
		0.0f,//FLOAT DepthBiasClamp;
		0.0f,//FLOAT SlopeScaledDepthBias;
		TRUE,//BOOL DepthClipEnable;
		FALSE,//BOOL ScissorEnable;
		TRUE,//BOOL MultisampleEnable;
		FALSE//BOOL AntialiasedLineEnable;        
	};
	ID3D11RasterizerState* pRS = nullptr;
	hr = g_pd3dDevice->CreateRasterizerState(&drd, &pRS);
	bug_on( FAILED( hr ));
	g_pImmediateContext->RSSetState(pRS);

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

float getPt( float n1, float n2, float perc )
{
	float diff = n2 - n1;
	return n1 + ( diff * perc );
}

DECLARE_MODULE_NAME("dx11");

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

int g_curBufferIdx = 0;

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

	D3D11_MAPPED_SUBRESOURCE mappedResource;
	xMemZero(mappedResource);

	auto*   simple      = g_DynVertBuffers[g_curBufferIdx].Simple;
	UINT    stride      = sizeof(SimpleVertex);
	UINT    offset      = 0;

	g_pImmediateContext->Map(simple, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	xMemCopy(mappedResource.pData, vertices, sizeof(vertices));
	g_pImmediateContext->Unmap(simple, 0);

	g_pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	g_pImmediateContext->IASetVertexBuffers(0, 1, &g_DynVertBuffers[g_curBufferIdx].Simple, &stride, &offset);
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
	g_pImmediateContext->VSSetShader(g_pVertexShader, nullptr, 0);
	//g_pImmediateContext->VSSetConstantBuffers(0, 1, &g_pConstantBuffer);
	g_pImmediateContext->PSSetShader(g_pPixelShader, nullptr, 0);
	g_pImmediateContext->DrawIndexed((numVertexesPerCurve*4), 0,  0);

	//g_pSwapChain->Present(1, DXGI_SWAP_EFFECT_SEQUENTIAL);
	g_pSwapChain->Present(0, 0);
	g_curBufferIdx = (g_curBufferIdx+1) % BackBufferCount;
	xThreadSleep(20);
}

int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
	s_mtx_unilogger.Create();


	if (FAILED(InitWindow(hInstance, nCmdShow)))
		return 0;

	if (FAILED(InitDevice()))
	{
		CleanupDevice();
		return 0;
	}

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

	for (int i=0; i<BackBufferCount; ++i) {
		D3D11_SUBRESOURCE_DATA InitData;
  
		D3D11_BUFFER_DESC bd;
		xMemZero(bd);
		bd.Usage			= D3D11_USAGE_DYNAMIC;
		bd.ByteWidth		= sizeof(SimpleVertex) * SimpleVertexBufferSize;
		bd.BindFlags		= D3D11_BIND_VERTEX_BUFFER;
		bd.CPUAccessFlags	= D3D11_CPU_ACCESS_WRITE;

		xMemZero(InitData);
		//InitData.pSysMem = vertices;

		auto hr = g_pd3dDevice->CreateBuffer(&bd, nullptr, &g_DynVertBuffers[i].Simple);
		bug_on (FAILED(hr));


		xMemZero(bd);
		bd.Usage			= D3D11_USAGE_DEFAULT;
		bd.ByteWidth		= sizeof(indices);
		bd.BindFlags		= D3D11_BIND_INDEX_BUFFER;
		bd.CPUAccessFlags	= 0;

		xMemZero(InitData);
		InitData.pSysMem	= indices;

		hr = g_pd3dDevice->CreateBuffer( &bd, &InitData, &g_pIndexBuffer );
		bug_on (FAILED(hr));
	}


	// Set index buffer
	g_pImmediateContext->IASetIndexBuffer( g_pIndexBuffer, DXGI_FORMAT_R16_UINT, 0 );

	// Main message loop
	MSG msg = { 0 };
	while (WM_QUIT != msg.message)
	{
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		} else
		{
			Render();
		}
	}

	CleanupDevice();

	return (int)msg.wParam;
}