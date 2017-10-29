#include "PCH-rpgcraft.h"

#include "msw-redtape.h"

#include "x-string.h"
#include "x-gpu-ifc.h"
#include "x-thread.h"
#include "x-host-ifc.h"
#include "x-chrono.h"
#include "x-pad.h"

#include "appConfig.h"
#include "ajek-script.h"
#include "Scene.h"

#include "imgui.h"

#include <direct.h>		// for _getcwd()

DECLARE_MODULE_NAME("winmain");

extern void			LogHostInit();
extern void			MSW_InitChrono();

HINSTANCE               g_hInst					= nullptr;
HWND                    g_hWnd					= nullptr;

//--------------------------------------------------------------------------------------
// WINDOWS BOILERPLATE
//--------------------------------------------------------------------------------------
LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC hdc;

    ImGuiIO& io = ImGui::GetIO();

	switch (msg)
	{
		case WM_PAINT:
			hdc = BeginPaint(hWnd, &ps);
			EndPaint(hWnd, &ps);
		break;

		case WM_DESTROY:
			Scene_ShutdownThreads();
			dx11_CleanupDevice();
			PostQuitMessage(0);
		break;

		case WM_SYSKEYDOWN: {
			WPARAM param = wParam;
			char c = MapVirtualKey (param, MAPVK_VK_TO_CHAR);
			auto mod = Host_GetKeyModifierInMsg();

			if ((c == 'W' || c == 'w') && mod.Alt()) {
				g_gpu_ForceWireframe = !g_gpu_ForceWireframe;
			}

			if (wParam < 256)
				io.KeysDown[wParam] = 1;
		} break;

		case WM_KEYDOWN: {

			WPARAM param = wParam;
			char c = MapVirtualKey (param, MAPVK_VK_TO_CHAR);

			if (c == 'R' || c == 'r') {
				Scene_PostMessage(SceneMsg_Reload, 0);
				Scene_PostMessage(SceneMsg_StartExec, SceneStopReason_ScriptError);
			}

			if (wParam < 256)
				io.KeysDown[wParam] = 1;
		} break;

		case WM_KEYUP:
		case WM_SYSKEYUP:
			if (wParam < 256)
				io.KeysDown[wParam] = 0;
        break;

		// --------------------------------------------------------------------------------
		// UI-style User-input (mouse/keyboard), which use buffered input from Windows
		// instead of per-frame polling used by the gameplay input system.

		case WM_MOUSEWHEEL: {
			io.MouseWheel += GET_WHEEL_DELTA_WPARAM(wParam) > 0 ? +1.0f : -1.0f;
		} break;

		case WM_MOUSEMOVE: {
			io.MousePos.x = (signed short)(lParam);
			io.MousePos.y = (signed short)(lParam >> 16);
		} break;

		case WM_LBUTTONDOWN:
		case WM_RBUTTONDOWN:
		case WM_MBUTTONDOWN:
		{
			int button = 0;
			if (msg == WM_LBUTTONDOWN) button = 0;
			if (msg == WM_RBUTTONDOWN) button = 1;
			if (msg == WM_MBUTTONDOWN) button = 2;
			SetCapture(hWnd);
			io.MouseDown[button] = true;
		} break;

		case WM_LBUTTONUP:
		case WM_RBUTTONUP:
		case WM_MBUTTONUP:
		{
			int button = 0;
			if (msg == WM_LBUTTONUP) button = 0;
			if (msg == WM_RBUTTONUP) button = 1;
			if (msg == WM_MBUTTONUP) button = 2;
			io.MouseDown[button] = false;
			ReleaseCapture();
		} break;

		case WM_CHAR: {
			// You can also use ToAscii()+GetKeyboardState() to retrieve characters.
			if (wParam > 0 && wParam < 0x10000)
				io.AddInputCharacter((unsigned short)wParam);
		} break;

		default:
			return DefWindowProc(hWnd, msg, wParam, lParam);
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
	HostClockTick::Init();
	MSW_InitChrono();
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

	AjekScript_InitAlloc();
	g_pkg_config_filename = "config-package-msw.lua";
	LoadPkgConfigFromMain(g_scriptEnv);

	// -----------------------------------------------------------
	// Init message recievers asap
	//   .. so that things that might want to queue msgs during init can do so...
	Scene_InitMessages();
	// -----------------------------------------------------------

	if (FAILED(InitWindow(hInstance, nCmdShow)))
		return 0;

	dx11_InitDevice();

	KPad_SetMapping(g_kpad_btn_map_default);
	KPad_SetMapping(g_kpad_axs_map_default);

	// Drain the message queue first before starting game threads.
	// No especially good reason for this -- there's just a bunch of poo in the windows
	// message queue after window creation and stuff, and I like to drain it all in as
	// synchronous an environment as possible.  --jstine

	if (Msw_DrainMsgQueue()) {

		KPad_CreateThread();
		Scene_CreateThreads();

		// Main message loop
		// Just handle messages here.  Don't want the msg loop doing anything that might take more
		// than a couple milliseconds.  Long jobs should always be dispatched to other threads.

		while (Msw_DrainMsgQueue()) {
			WaitMessage();
		}
	}
	Scene_ShutdownThreads();
	dx11_CleanupDevice();

	return 0;
}

// --------------------------------------------------------------------------------------
bool xIsDebuggerAttached()
{
	return ::IsDebuggerPresent();
}

void xOutputStringError(const char* str)
{
#if MSW_ENABLE_DEBUG_OUTPUT
	if (::IsDebuggerPresent())
		OutputDebugStringA(str);
	else
#endif
	{
		fputs(str, stderr);
	}
}

void xOutputString(const char* str)
{
#if MSW_ENABLE_DEBUG_OUTPUT
	if (::IsDebuggerPresent())
		OutputDebugStringA(str);
	else
#endif
	{
		fputs(str, stdout);
	}
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
