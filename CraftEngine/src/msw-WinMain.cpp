#include "PCH-rpgcraft.h"

#include "msw-redtape.h"

#include "x-string.h"
#include "x-stdfile.h"
#include "x-thread.h"
#include "x-gpu-ifc.h"
#include "x-host-ifc.h"
#include "x-chrono.h"
#include "x-pad.h"
#include "fmod-ifc.h"

#include "appConfig.h"
#include "Scene.h"

#include "imgui.h"

#include <direct.h>     // for _getcwd()

extern void         LogHostInit();
extern void         MSW_InitChrono();
extern VirtKey_t    ConvertFromMswVK( UINT key );

HINSTANCE               g_hInst                 = nullptr;
HWND                    g_hWnd                  = nullptr;

static INT64            g_Time = 0;
static INT64            g_TicksPerSecond = 0;

static xCountedSemaphore    s_sem_SettingsDirtied;
static xMutex               s_mtx_saved_by_app;
static volatile s32         s_settings_dirty;
static xString              s_settings_content_KnownVersion;
static xString              s_settings_content_NewVersion;

void MarkUserSettingsDirty()
{
    if (!AtomicExchange(s_settings_dirty, 1)) {
        s_sem_SettingsDirtied.PostIfNeeded();
    }
}

static bool s_isMinimized = false;
static bool s_isMaximized = false;
static bool s_msg_moved   = false;
static bool s_msg_sized   = false;

extern bool SaveDesktopSettings(bool isMarkedDirty);

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
        case WM_MOVE: {
            if (!s_isMinimized) {
                MarkUserSettingsDirty();
            }
        } break;

        case WM_SIZE: {
            s_isMinimized = (wParam & SIZE_MINIMIZED);
            s_isMaximized = (wParam & SIZE_MAXIMIZED);
            if (!s_isMinimized) {
                MarkUserSettingsDirty();
            }
        } break;

        case WM_PAINT:
            hdc = BeginPaint(hWnd, &ps);
            EndPaint(hWnd, &ps);
        break;

        case WM_DESTROY:
            SaveDesktopSettings(s_settings_dirty);
            Scene_ShutdownThreads();
            dx11_CleanupDevice();
            PostQuitMessage(0);
        break;

        case WM_SYSKEYDOWN: {
            Scene_PostMessage(SceneMsg_KeyDown, ConvertFromMswVK(wParam));
            char c = MapVirtualKey (wParam, MAPVK_VK_TO_CHAR);
            auto mod = Host_GetKeyModifierInMsg();

            if ((c == 'W' || c == 'w') && mod.Alt()) {
                g_gpu_ForceWireframe = !g_gpu_ForceWireframe;
            }
        } break;

        case WM_KEYDOWN: {
            Scene_PostMessage(SceneMsg_KeyDown, ConvertFromMswVK(wParam));

            char c = MapVirtualKey (wParam, MAPVK_VK_TO_CHAR);

            if (c == 'R' || c == 'r') {
                Scene_PostMessage(SceneMsg_Reload, 0);
                Scene_PostMessage(SceneMsg_StartExec, SceneStopReason_ScriptError);
            }
        } break;

        case WM_KEYUP:
        case WM_SYSKEYUP:
            Scene_PostMessage(SceneMsg_KeyUp, ConvertFromMswVK(wParam));
        break;

        case WM_CHAR: {
            // UI-style User-input (mouse/keyboard), which use buffered input from Windows
            // instead of per-frame polling used by the gameplay input system.
            // You can also use ToAscii()+GetKeyboardState() to retrieve characters.
            if (wParam > 0 && wParam < 0x10000)
                Scene_PostMessage(SceneMsg_KeyChar, (unsigned short)wParam);
        } break;

        case WM_MOUSEWHEEL: {
            double delta = GET_WHEEL_DELTA_WPARAM(wParam) / 120.f;
            Scene_PostMessage(SceneMsg_MouseWheelDelta, (sptr&)delta);
        } break;


        case WM_LBUTTONDOWN:
        case WM_RBUTTONDOWN:
        case WM_MBUTTONDOWN:
        {
            // Mouse capture is done via the message pump since it runs on its own "safe" thread
            // which is at low risk for deadlock and should be quick to respond to the user even
            // if the game loop is hung up for some reason.  Important since capturing the mouse
            // is sort of one of those "denial of UI service" things if it's not released. --jstine
            SetCapture(hWnd);
        } break;

        case WM_LBUTTONUP:
        case WM_RBUTTONUP:
        case WM_MBUTTONUP:
        {
            // no value to track capture state here -- errant release messages are rare so just
            // call ReleaseCapture() unconditionally. --jstine
            ReleaseCapture();
        } break;

        default:
            return DefWindowProc(hWnd, msg, wParam, lParam);
    }

    return 0;
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

//std::vector<RECT>  s_msw_monitors;
//BOOL MonitorEnumerator(HMONITOR monitor, HDC hdc, LPRECT rect, LPARAM lparam)
//{
//  log_host("MonitorEnumerator: rect = %d, %d, %d, %d", rect->left, rect->top, rect->right, rect->bottom);
//  s_msw_monitors.push_back(*rect);
//  return true;
//}

template< typename T >
void table_get_xy(T& dest, LuaTableScope& table)
{
    auto xs = table.get_s32("x");
    auto ys = table.get_s32("y");

    if (xs.isnil()) { xs = table.get_s32(1); }
    if (ys.isnil()) { ys = table.get_s32(2); }

    if (xs.isnil() || ys.isnil()) {
        bug_qa("Invalid table layout for expected Int2 parameter.");
        // Todo: expose a ThrowError that pushes message onto Lua stack and invokes longjmp.
        // Pre-req: must have a single global lua state for all scene magic.
        //ThrowError
    }

    dest.x = xs;
    dest.y = ys;
}

template< typename T >
bool table_get_xy(T& dest, LuaTableScope& table, const char* subtable)
{
    bug_on(!subtable);
    if (auto& subtab = table.get_table(subtable)) {
        table_get_xy(dest, subtab);
        return true;
    }
    return false;
}

template< typename T >
void table_get_rect(T& dest, LuaTableScope& table)
{
    if (auto& subtab = table.get_table(1)) {
        table_get_xy(dest.xy, subtab);
    }

    if (auto& subtab = table.get_table(2)) {
        table_get_xy(dest.zw, subtab);
    }
}

static RECT s_lastknown_window_rect     = { };

static bool validate_window_pos(RECT& window, bool tryFix)
{
    // This logic mimics Win10's built-in movement restrictions, which aren't applied when using
    // ::ShowWindow() -- so we have to do it manually in case the inputs from the config file are
    // garbage.

    // * the "top" of a window should always be visible on load, because it's important for being
    //   able to grab and move the window.
    // * The rest of the window position isn't too important, however at least 240px or 1/3rd of
    //   the window along the X (horizontal) axis should be visble, otherwise it might be hard to
    //   grab the title bar.

    // get the work area of the monitor that best matches our window position:
    auto hMonitor = ::MonitorFromRect(&window, MONITOR_DEFAULTTONEAREST);
    MONITORINFO  mi = {};
    mi.cbSize = sizeof(mi);
    GetMonitorInfo(hMonitor, &mi);
    RECT best_monitor = mi.rcWork;

    // Determine the non-client area cleverly, without having to go through all the stupid
    // windows metrics APIs:

    RECT nonclientarea = { 0,0,1,1 };
    POINT titlebarSize = { 8, 8    };       // default assumption in case AdjustWindowRect() fails for any reaosn
    auto style = ::GetWindowLong(g_hWnd, GWL_STYLE);
    if (::AdjustWindowRect(&nonclientarea, style, FALSE)) {
        titlebarSize = { -nonclientarea.left, -nonclientarea.top };
    }

    RECT dest;
    RECT window_important_area = {  window.left, window.top, window.right, window.top + titlebarSize.y };
    int importantWidth = min(240, window.right-window.left);

    if (::IntersectRect(&dest, &window_important_area, &best_monitor)) {
        bool horizCheckOk = (dest.bottom - dest.top) >= titlebarSize.y;
        bool vertCheckOk  = (dest.right - dest.left) >= importantWidth;
        if (horizCheckOk && vertCheckOk) {
            // Seems OK, let the position through unmodified...
            return true;
        }
    }

    if (!tryFix) {
        return false;
    }

    POINT moveby = {
        best_monitor.left       - window.left,
        best_monitor.top        - window.top,
    };

    window.left     += moveby.x;
    window.top      += moveby.y;
    window.right    += moveby.x;
    window.bottom   += moveby.y;

    return true;
}

void ApplyDesktopSettings()
{
    int2 client_pos     = g_settings_hostwnd.client_pos;
    int2 client_size    = g_settings_hostwnd.client_size;

    // if client pos was never specified by config, then use whatever was assigned to our window by
    // the system when it was created...

    if (!g_settings_hostwnd.has_client_pos) {
        RECT cur_rect;
        if (::GetWindowRect(g_hWnd, &cur_rect)) {
            client_pos = { cur_rect.left, cur_rect.top };
        }
    }

    // always good idea to make sure window startup / init messages have been processed before reading
    // or setting the window size/pos.
    Msw_DrainMsgQueue();

    RECT rc = { client_pos.x, client_pos.y, client_pos.x + client_size.u, client_pos.y + client_size.v };
    auto style = ::GetWindowLong(g_hWnd, GWL_STYLE);
    ::AdjustWindowRect(&rc, style, FALSE);

    // always validate window pos even if has_pos is false, since it also validates window size.
    validate_window_pos(rc, true);
    u32 posflags = SWP_NOOWNERZORDER | SWP_NOZORDER;
    ::SetWindowPos(g_hWnd, nullptr, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, posflags);
}

static void UpdateLastKnownWindowPosition()
{
    if (!s_isMinimized && !s_isMaximized) {
        RECT rc;
        ::GetClientRect(g_hWnd, &rc);

        POINT topleft           = { rc.left, rc.top };
        POINT bottomright       = { rc.right, rc.bottom };

        ::ClientToScreen(g_hWnd, &topleft);
        ::ClientToScreen(g_hWnd, &bottomright);

        RECT newpos = { topleft.x, topleft.y, bottomright.x, bottomright.y };

        if (validate_window_pos(newpos, false)) {
            s_lastknown_window_rect     = newpos;
        }
    }
}

static void AppendDesktopSettings(xString& dest)
{
    // This version ensures the client position stays consistent even if the decaling sizes change between sessions,
    // (eg, user changes window title bar sizes or similar) -- and generally makes more sense from a human-readable
    // perspective.

    UpdateLastKnownWindowPosition();
    dest.AppendFmt("window-client-pos  = %d,%d\n",
        s_lastknown_window_rect.left, s_lastknown_window_rect.top
    );

    dest.AppendFmt("window-client-size = %d,%d\n",
        g_client_size_pix.x, g_client_size_pix.y
    );
}

bool SaveDesktopSettings(bool isMarkedDirty)
{
    s_mtx_saved_by_app.Lock();  Defer(s_mtx_saved_by_app.Unlock());

    // NewVersion is static (non-local) to avoid redundant heap alloc.
    s_settings_content_NewVersion.Clear();
    AppendDesktopSettings(s_settings_content_NewVersion);

    if (s_settings_content_NewVersion == s_settings_content_KnownVersion) {
        return false;
    }

    if (!isMarkedDirty) {
        // periodic save detected unexpected changes, which means there's something
        // changing in the settings env that's not issuing dirty pings...
        // TODO : print a diff of the specific lines that changed, so we can identify what needs
        //        to be fixed to call the Dirty function.
        warn_host( "Unexpected saved settings occured." );
    }

    xCreateDirectory(".ajek");
    if (FILE* f = xFopen(".ajek/saved-by-app.cli", "wb")) {
        fputs("# Machine-generated user settings file.\n",f);
        fputs("# Any human modifications here will be lost!\n\n",f);
        fputs(s_settings_content_NewVersion, f);
        s_settings_content_KnownVersion = s_settings_content_NewVersion;
        fclose(f);
    }
    return true;
}

// Optional thread to ping settings periodically, to cover handling settings that aren't correctly
// pinging s_sem_SettingsDirtied.
void* PingSaveSettingsThread(void*)
{
    while (1) {
        s_sem_SettingsDirtied.PostIfNeeded();
        xThreadSleep(10000);
    }
}

void* SaveSettingsThread(void*)
{
    while (1) {
        s_sem_SettingsDirtied.Wait();
        auto isMarkedDirty = (AtomicExchange(s_settings_dirty,0) >= 1);
        if (SaveDesktopSettings(isMarkedDirty)) {
            xThreadSleep(3000);
        }
        else {
            xThreadSleep(250);
        }
    }
}


HRESULT InitWindow(HINSTANCE hInstance, int nCmdShow)
{
    // note: current unused: MonitorFromRect() combined with GetMonitorInfo() is a much more useful
    // set of functions.

    //s_msw_monitors.clear();
    //EnumDisplayMonitors(nullptr, nullptr, MonitorEnumerator, 0);

    // Register class
    WNDCLASSEX wcex;
    wcex.cbSize         = sizeof(WNDCLASSEX);
    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(NULL, IDI_APPLICATION); //LoadIcon(hInstance, (LPCTSTR)IDI_TUTORIAL1);
    wcex.hIconSm        = LoadIcon(NULL, IDI_APPLICATION); //LoadIcon(wcex.hInstance, (LPCTSTR)IDI_TUTORIAL1);
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName   = nullptr;
    wcex.lpszClassName  = L"RpgCraftGame";
    if (!RegisterClassEx(&wcex))
        return E_FAIL;

    // Create window
    g_hInst = hInstance;
    RECT rc = { 0, 0, 1280, 720 };
    AdjustWindowRect(&rc, WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX, FALSE);
    g_hWnd = CreateWindow(L"RpgCraftGame", L"RPG Craft: Bloody Spectacular Edition",
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, nullptr, nullptr, hInstance,
        nullptr);
    if (!g_hWnd)
        return E_FAIL;

    ShowWindow(g_hWnd, nCmdShow);

    return S_OK;
}

xString Host_GetCWD()
{
    char   buff[1024];
    char*  ret = _getcwd(buff, 1024);
    return ret ? xString(buff) : xString();
}

xString Host_GetFullPathName(const xString& relpath)
{
    if (xPathIsAbsolute(relpath)) {
        return relpath;
    }

    wchar_t meh[4096];
    auto len = ::GetFullPathNameW(toUTF16(relpath).wc_str(), 4095, meh, nullptr);
    if (len == 0) {
        warn_host("GetFullPathNameW('%s') failed.", relpath.c_str());
        return relpath;
    }
    return meh;
}

int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
    s_sem_SettingsDirtied.Create("SettingsDirtied");
    s_mtx_saved_by_app.Create("SettingsSavedByApp");

    HostClockTick::Init();
    MSW_InitChrono();
    LogHostInit();

    fmod_CheckLib();
    fmod_InitSystem();

    // an assist to microsoft for being kind enough to just provide these as globals already.
    auto g_argc = __argc;

    #if 0       // diag thing
    for (int a=1; a<g_argc; ++a) {
        OutputDebugStringA( __argv[a] );
        OutputDebugStringA( TEXT("\n") );
    }
    #endif

    // look for some first-chance switches in the cli.
    // Anything prefixed with a double-dash is applied _before_ the package config.
    // Anything without double-dash is processed _after_ the package config.
    // This allows developers to specify CLI options that override the package config.

    for (int a=1; a<g_argc; ++a) {
#ifdef UNICODE
        xString utf8(__wargv[a]);
#else
        xString utf8(__argv[a]);
#endif

        if (utf8.StartsWith("--")) {
            CliParseOption(utf8);
        }
    }

    CliParseFromFile("config-package-msw.cli.txt");

    for (int a=1; a<g_argc; ++a) {
#ifdef UNICODE
        xString utf8(__wargv[a]);
#else
        xString utf8(__argv[a]);
#endif

        if (!utf8.StartsWith("--")) {
            CliParseOption(utf8);
        }
    }

    AjekScript_InitAlloc();

    // -----------------------------------------------------------
    // Init message recievers asap
    //   .. so that things that might want to queue msgs during init can do so...
    Scene_InitMessages();
    // -----------------------------------------------------------

    if (FAILED(InitWindow(hInstance, false)))
        return 0;

    ApplyDesktopSettings();
    dx11_InitDevice();
    ShowWindow(g_hWnd, true);
    UpdateLastKnownWindowPosition();

    KPad_SetMapping(g_kpad_btn_map_default);
    KPad_SetMapping(g_kpad_axs_map_default);

    // TODO: pulled from imgui - repeal and replace with existing msw-chrono stuff.
    if (!QueryPerformanceFrequency((LARGE_INTEGER *)&g_TicksPerSecond))
        return false;
    if (!QueryPerformanceCounter((LARGE_INTEGER *)&g_Time))
        return false;

    // Drain the message queue first before starting game threads.
    // No especially good reason for this -- there's just a bunch of poo in the windows
    // message queue after window creation and stuff, and I like to drain it all in as
    // synchronous an environment as possible.  --jstine

    thread_t s_thr_save_settings;
    thread_t s_thr_save_settings_ping;
    thread_create(s_thr_save_settings, SaveSettingsThread, "SaveSettings");
    thread_create(s_thr_save_settings_ping, PingSaveSettingsThread, "PingSaveSettings");

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

void xOutputString(const char* str, FILE* std_fp)
{
#if MSW_ENABLE_DEBUG_OUTPUT
    if (::IsDebuggerPresent())
        OutputDebugStringA(str);
    else
#endif
    {
        fputs(str, std_fp);
    }
}

static __ai assert_t DoAssertionDialog( const xString& title, const xString& content )
{
//  AtomicInc( s_open_assertion_counter );

    auto result = MessageBox(nullptr,
        toUTF16(content).wc_str(), toUTF16(title).wc_str(), MB_ABORTRETRYIGNORE
    );

//  AtomicDec( s_open_assertion_counter );

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
//  if (Host_IsUsingCLI())      return assert_break;
    if (IsDebuggerPresent())    return assert_break;

    return DoAssertionDialog( title, message + "\n\nAssertionContext:\n" + context );
}

void _hostImpl_ImGui_NewFrame()
{
    ImGuiIO& io = ImGui::GetIO();

    // Setup display size (every frame to accommodate for window resizing)
    RECT rect;
    GetClientRect(g_hWnd, &rect);
    io.ImeWindowHandle  = g_hWnd;
    io.DisplaySize      = ImVec2((float)(rect.right - rect.left), (float)(rect.bottom - rect.top));

    // Setup time step
    INT64 current_time;
    QueryPerformanceCounter((LARGE_INTEGER *)&current_time);
    io.DeltaTime = (float)(current_time - g_Time) / g_TicksPerSecond;
    g_Time = current_time;

    // io.MouseWheel : filled by WM_MOUSEWHEEL events

    // Read keyboard modifiers inputs
    io.KeyCtrl  = (::GetAsyncKeyState(VK_CONTROL)   & 0x8000) != 0;
    io.KeyShift = (::GetAsyncKeyState(VK_SHIFT  )   & 0x8000) != 0;
    io.KeyAlt   = (::GetAsyncKeyState(VK_MENU   )   & 0x8000) != 0;
    io.KeySuper = (::GetAsyncKeyState(VK_LWIN   )   & 0x8000) != 0;

    // Read Mouse position.
    // Use polling instead of WM_MOUSEMOVE since the windows msg pump isn't on this thread.

    auto mouseState = HostMouseImm_GetState();
    io.MousePos = ImVec2(-FLT_MAX,-FLT_MAX);

    if (mouseState.isValid) {
        io.MousePos = float2(mouseState.clientPos);

        io.MouseDown[0] = mouseState.pressed.LBUTTON;
        io.MouseDown[1] = mouseState.pressed.RBUTTON;
        io.MouseDown[2] = mouseState.pressed.MBUTTON;

        // Set OS mouse position if requested last frame by io.WantMoveMouse flag (used when io.NavMovesTrue is enabled by user and using directional navigation)
        if (io.WantMoveMouse)
        {
            POINT pos = { (int)io.MousePos.x, (int)io.MousePos.y };
            ::ClientToScreen(g_hWnd, &pos);
            ::SetCursorPos(pos.x, pos.y);
        }

        // Hide OS mouse cursor if ImGui is drawing it
        if (io.MouseDrawCursor) {
            ::SetCursor(NULL);
        }
    }

    // Start the frame
    ImGui::NewFrame();
}
