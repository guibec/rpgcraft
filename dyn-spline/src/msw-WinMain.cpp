#include "msw-redtape.h"

#include "x-string.h"
#include "x-gpu-ifc.h"
#include "x-thread.h"
#include "x-host-ifc.h"

#include "ajek-script.h"
#include "Scene.h"


#include <direct.h>		// for _getcwd()

DECLARE_MODULE_NAME("winmain");

extern void			LogHostInit();

extern void			LoadPkgConfig(const xString& luaFile);

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

		case WM_KEYDOWN: {

			WPARAM param = wParam;
			char c = MapVirtualKey (param, MAPVK_VK_TO_CHAR);

			if (c == 'W' || c == 'w') {
				g_gpu_ForceWireframe = !g_gpu_ForceWireframe;
			}

			if (c == 'R' || c == 'r') {
				Scene_PostMessage(SceneMsg_Reload, 0);
			}

		}break;

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
	RECT rc = { 0, 0, 1280, 720 };
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

static inline bool _sopt_isWhitespace( char ch )
{
	return !ch
		|| (ch == '\r') 
		|| (ch == '\n')
		|| (ch == ' ') 
		|| (ch == '\t');
}

xString Host_GetCWD()
{
	char   buff[1024];
	char*  ret = _getcwd(buff, 1024);
	return ret ? xString(buff) : xString();
}

bool Msw_DrainMsgQueue() 
{
	MSG msg = { 0 };

	while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);

		if (msg.message == WM_QUIT) {
			return false;
		}
	}

	return true;
}

int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
	// 
	LogHostInit();

	// Tokenizer.
	//   * only care about double dash "--" anything lacking a double-dash prefix is an error.
	//   * all switches require assignment operator '=', eg:
	//        --option=value
	//   * Boolean toggles are required to specify --arg=0 or --arg=1
	//   * whitespace in options is disallowed
	//   * Whitespace is handled by the shell command line processor, so any whitespace is assumed part of the value.
	//   * environment variable expansion not supported (expected to be performed by shell)
	//
	// Windows-sepcific notes:
	//   * __argv has double-quotes already processed, but unfortunately Windows has some rather dodgy quotes parsing,
	//     so it's likely there'll be spurious quotes lying around when injecting Visual Studio EnvVars.  For now it's
	//     responsibility of user to fix those.
	//   * __argv does not process single quotes. These will be present and will result in malformed CLI syntax.
	//
	// The goal is to use lua for the bulk of cmdline parsing.  The only command line options to be
	// parsed here are things that we want to be applied *before* the lua engine has been started.
	//   * Configuration of log file things
	//   * config-local.lua override
	//   * visual studio script debug mode

	// an assist to microsoft for being kind enough to just provide these as globals already.

	auto g_argc = __argc;

	for (int a=1; a<g_argc; ++a) {
		//OutputDebugString( g_argv[a] );
		//OutputDebugString( TEXT("\n") );

#ifdef UNICODE
		xString utf8(__wargv[a]);
#else
		xString utf8(__argv[a]);
#endif
		char optmp[128] = { 0 };

		log_and_abort_on(!utf8.StartsWith("--"), "Invalid CLI option specified: %s", utf8.c_str());
		int readpos  = 2;
		int writepos = 0;
		for(;;) {
			log_and_abort_on(!utf8.data()[readpos] || readpos >= utf8.GetLength(),
				"Unexpected end of CLI option while searching for '='\n   Option Text: %s", utf8.c_str()
			);
			log_and_abort_on(_sopt_isWhitespace(utf8.data()[readpos]), 
				"Invalid whitespace detected in CLI lvalue: %s", utf8.c_str()
			);
			log_and_abort_on(writepos >= bulkof(optmp)-1, "CLI option text is too long!");

			if (utf8.data()[readpos] == '=') break;
			optmp[writepos] = utf8.data()[readpos];
			++writepos;
			++readpos;
		}
		log_and_abort_on(!writepos, "Invalid zero-length option: %s", utf8.c_str());
		optmp[writepos+1] = 0;
		xString val		(utf8.data() + readpos + 1);		// forward past '='
		xString option	(optmp);
		if (0) {
			// start of list
		}
		elif(option == "script-dbg-relpath") {
			// Visual Studio script debug mode:
			// Script error messages should be printed relative to the solution directory.
			AjekScript_SetDebugRelativePath(val);
		}
		elif (0) {
			// end of list.
		}
	}

	LoadPkgConfig("config-package-msw.lua");

	// -----------------------------------------------------------
	// Init message recievers asap
	//   .. so that things that might want to queue msgs during init can do so...
	Scene_InitMessages();
	// -----------------------------------------------------------

	if (FAILED(InitWindow(hInstance, nCmdShow)))
		return 0;

	dx11_InitDevice();

	// Drain the message queue first before starting game threads.
	// No especially good reason for this -- there's just a bunch of poo in the windows
	// message queue after window creation and stuff, and I like to drain it all in as
	// synchronous an environment as possible.  --jstine

	if (Msw_DrainMsgQueue()) {

		Scene_CreateThreads();

		// Main message loop
		// Just handle messages here.  Don't want the msg loop doing anything that might take more
		// than a couple milliseconds.  Long jobs should always be dispatched to other threads.

		while (Msw_DrainMsgQueue()) {
			WaitMessage();
		}
	}

	dx11_CleanupDevice();
	Scene_ShutdownMessages();

	return 0;
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
		xPrintLn( "*** DEBUG BREAK REQUESTED ***" );
		return assert_break;
	}
	else if (result == IDABORT)
	{
		xPrintLn( "*** TERMINATION REQUESTED ***" );
		return assert_terminate_app;
	}
	else if (result == IDIGNORE)
	{
		xPrintLn( "*** all assertions of this type ignored! ***" );
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
