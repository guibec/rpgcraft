#include "msw-redtape.h"

#include "x-string.h"

extern void			LogHostInit();
extern HRESULT		InitDevice();
extern void			CleanupDevice();


extern void			DoGameInit();
extern void			Render();

HINSTANCE               g_hInst					= nullptr;
HWND                    g_hWnd					= nullptr;

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


int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
	LogHostInit();

	if (FAILED(InitWindow(hInstance, nCmdShow)))
		return 0;

	if (FAILED(InitDevice()))
	{
		CleanupDevice();
		return 0;
	}

	DoGameInit();

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


// --------------------------------------------------------------------------------------
bool xIsDebuggerAttached()
{
	return !! ::IsDebuggerPresent();
}

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
