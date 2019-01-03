#include "PCH-rpgcraft.h"

#include "x-types.h"
#include "x-stl.h"
#include "x-assertion.h"
#include "x-string.h"
#include "x-thread.h"
#include "x-chrono.h"
#include "x-pad.h"
#include "x-host-ifc.h"
#include "x-gpu-ifc.h"
#include "x-ThrowContext.h"
#include "x-chrono.h"

#include "appConfig.h"
#include "ajek-script.h"

#include "Scene.h"
#include "Entity.h"
#include "DbgFont.h"

#include <queue>
#include <ctime>

DECLARE_MODULE_NAME("scene");

typedef std::queue<SceneMessage> SceneMessageList;

static thread_t             s_thr_scene_producer;
static xMutex               s_mtx_MsgQueue;
static xSemaphore           s_sem_thread_done;
static SceneMessageList     s_MsgQueue;
static u32                  s_scene_stopReason          = 0;
static u32                  s_scene_devExecMask         = 0xffffffff;
static bool                 s_scene_initialized         = false;
static bool                 s_scene_thread_running      = false;

static int                  s_scene_frame_count         = 0;
static bool                 s_scene_has_keyfocus = false;

// Local world time is time measured while the game is not in a paused state.
// This differs from network time, which will be based on either universal QPC results,
// or something roughly similar that at at least 5ms resolution.

HostClockTick               s_world_deltatime;          // in seconds.
HostClockTick               s_world_localtime;
HostClockTick               s_world_localtime_qpc_last_update;

HostClockTick               s_world_qpc_frametick;          // cached QPC value, once per frame (scene loop)


HostClockTick WorldTick_Now()
{
    return s_world_qpc_frametick;
}

bool Scene_IsKeyPressed(VirtKey_t vk_code)
{
    if (!s_scene_has_keyfocus)      { return false; }
    return Host_IsKeyPressedGlobally(vk_code);
}

int Scene_GetFrameCount()
{
    return s_scene_frame_count;
}

void Scene_InitMessages()
{
    s_mtx_MsgQueue.Create("SceneMsgQueue");
    s_sem_thread_done.Create();
}

// Provided for debugging, not meant to be called in normal operation.
// By design we want mutexes to stay live until the process dies, that way any dependent
// threads that might try to lock/unlock the mutex won't crash while shutdown progresses.
void Scene_CleanupThreadObjects()
{
    s_mtx_MsgQueue.Delete();
    s_sem_thread_done.Delete();
}

bool Scene_HasPendingMessages()
{
    xScopedMutex lock(s_mtx_MsgQueue);
    return !s_MsgQueue.empty();
}

void Scene_PostMessage(SceneMessageId msgId, sptr payload)
{
    xScopedMutex lock(s_mtx_MsgQueue);
    s_MsgQueue.push( SceneMessage(msgId, payload) );
}

static __ai u32 _getStopReason(u64 payload)
{
    u32 result = _NCU32(payload);
    if (!result) result = SceneStopReason_Developer;
    return result;
}

void Scene_DrainMsgQueue()
{
    bool processed  = false;        // flag if any msgs are processed at all.

    xScopedMutex lock(s_mtx_MsgQueue);
    while (!s_MsgQueue.empty())
    {
        SceneMessage msg = s_MsgQueue.front();
        s_MsgQueue.pop();
        lock.Unlock();
        processed = true;

        ImGuiIO& io = ImGui::GetIO();
        switch(msg.msgid)
        {
            case SceneMsg_StopExec: {
                s_scene_stopReason   |=  _getStopReason(msg.payload);
            } break;

            case SceneMsg_StartExec: {
                s_scene_stopReason   &= ~_getStopReason(msg.payload);
            } break;

            case SceneMsg_ToggleExec: {
                s_scene_stopReason   ^=  _getStopReason(msg.payload);
            } break;

            case SceneMsg_StepExec: {
                s_scene_stopReason   ^= SceneStopReason_Developer;
                //step_exec           = !s_scene_stopReason;
            } break;

            case SceneMsg_Reload: {
                s_scene_initialized = false;
            } break;

            case SceneMsg_Shutdown: {
                s_scene_stopReason |= _SceneStopReason_Shutdown;
            } break;

            case SceneMsg_KeyDown: {
                if (msg.payload < bulkof(io.KeysDown)) {
                    io.KeysDown[msg.payload] = 1;
                }
            } break;

            case SceneMsg_KeyUp: {
                if (msg.payload < bulkof(io.KeysDown)) {
                    io.KeysDown[msg.payload] = 0;
                }
            } break;

            case SceneMsg_KeyChar: {
                io.AddInputCharacter(msg.payload);
            } break;

            case SceneMsg_MouseWheelDelta: {
                float delta = float((double&)msg.payload);
                io.MouseWheel += msg.payload;
            } break;
        }
    }
}

void Scene_ForceUpdateScripts()
{
}

__ai bool SceneInitialized() {
    return s_scene_initialized;
}

__ni void SceneInit()
{
    if (s_scene_initialized) {
        return;
    }

    g_scriptEnv.NewState();
    g_scriptEnv.BindThrowContext(g_ThrowCtx);

    EntityManager_Reset();

    x_try() {
        g_scriptEnv.LoadModule(g_pkg_config_filename);
        AjekScript_LoadConfiguration(g_scriptEnv);

        DbgFont_LoadInit    ();
        Scene_TryLoadInit   ();
        s_scene_initialized = true;
    }
    x_catch() {
        case xThrowModule_GPU:
        case xThrowModule_Script:
        case xThrowModule_Bridge:
            xPrintLn("");
            g_ThrowCtx.PrintLastError();

            if (!xIsDebuggerAttached()) {
                x_abort("Application aborted during InitScene.");
            }

            Scene_PostMessage(SceneMsg_StopExec, SceneStopReason_ScriptError);
            AjekScript_PrintDebugReloadMsg();
        break;

        default:
            bug_qa("Unhandled exception code = %d", x_exccode());
        break;
    }
    x_finalize() {
    }

}

static bool s_repeating_char[128];

static bool IsKeyPressedGlobally(char c)
{
    if (IsKeyPressedGlobally(c)) {
        bool result = !s_repeating_char[c];
        s_repeating_char[c] = 1;
        return result;
    }
    else {
        s_repeating_char[c] = 0;
    }
}

static void* GlobalKeyboardThreadProc(void*)
{
    xMemZero(s_repeating_char);

    auto modifiers  = Host_GetKeyModifier();

    // Repeater detection should be bound to the key and not to modifiers.
    // therefore always check key FIRST, and then modifiers within...

    if (IsKeyPressedGlobally('R')) {
        if (modifiers.CtrlWin()) {
            // todo.
        }
    }
}

extern void DevUI_DevControl        ();

void DevUI_Clocks()
{
    if (!ImGui::Begin("Clocks")) return;

    auto secs  = time(nullptr);

    // calc local time and the current timezone.
    // no function to return the current timezone, so use gmtime to calculate it ourselves...
    tm   localtm, gmtm;
    localtime_s (&localtm,  &secs);
    gmtime_s    (&gmtm,     &secs);

    auto loc_secs = mktime(&localtm);
    auto gm_secs  = mktime(&gmtm);
    auto tz_shift_secs  = loc_secs - gm_secs;

    ImGui::Value("proctime  ", HostClockTick::Now().asSeconds(), "%7.2fs");
    ImGui::Value("worldtime ", s_world_localtime.asSeconds(), "%7.2fs");
    ImGui::Value("deltatime ", s_world_deltatime.asMilliseconds(), "%7.02fms");
    ImGui::NewLine();
    ImGui::Text("localtime  : %02d:%02d:%02d %02d:%02d   (%04d-%02d-%02d)",
        localtm.tm_hour, localtm.tm_min, localtm.tm_sec,
        (tz_shift_secs / 3600), ((tz_shift_secs /60) % 60),
        localtm.tm_year+1900, localtm.tm_mon+1, localtm.tm_mday
    );

    ImGui::End();
}

static void* SceneProducerThreadProc(void*)
{
    Host_ImGui_Init();

    while(1)
    {
        Host_ImGui_NewFrame();
        DbgFont_NewFrame();

        // Timer Features!
        //  - Changes to Pause/Stop status occur during the msg queue
        //  - World Timer should not count time when the game has been paused
        //     * this includes time spent in the sleeping or vsync'ing since prev frame
        //  - Delta is not applied until scene is resumed, this allows the game delta state
        //    to be identical for paused scene reloads.

        Scene_DrainMsgQueue();
        s_scene_has_keyfocus  = !ImGui::GetIO().WantCaptureKeyboard && Host_HasWindowFocus();

        s_world_qpc_frametick = HostClockTick::Now();

        if (!s_scene_stopReason && s_world_localtime_qpc_last_update.asTicks() != 0) {
            s_world_deltatime  = WorldTick_Now() - s_world_localtime_qpc_last_update;
            s_world_localtime += s_world_deltatime;
        }
        s_world_localtime_qpc_last_update = HostClockTick::Now();

        if (Scene_HasStopReason(_SceneStopReason_Shutdown)) {
            log_host("SceneThread has been shutdown.");
            break;
        }

        DevUI_DevControl();
        DevUI_Clocks();

        if (Scene_HasStopReason(SceneStopReason_ScriptError)) {
            dx11_BeginFrameDrawing();
            dx11_SetRasterState(GPU_Fill_Solid, GPU_Cull_None, GPU_Scissor_Disable);
            dx11_ClearRenderTarget(g_gpu_BackBuffer, GPU_Colors::MidnightBlue);
            DbgFont_SceneRender();
            ImGui::Render();
            dx11_SubmitFrameAndSwap();
            xThreadSleep(32);
            s_world_localtime_qpc_last_update = HostClockTick::Now();
            continue;
        }

        if (Scene_HasStopReason(SceneStopReason_Background | SceneStopReason_HostDialog)) {
            xThreadSleep(32);
            s_world_localtime_qpc_last_update = HostClockTick::Now();
            continue;
        }

        if (!SceneInitialized()) {
            SceneInit();
            s_world_localtime_qpc_last_update = HostClockTick::Now();
        }

        if (!SceneInitialized()) {
            // init failed, don't render scene.
            // There should be a SceneStopReason set at this point to prevent constant
            // initialization loop feedback.  But we can't bug on it here since the stop
            // reason change is probably in the message queue and hasn't been applied yet.
            xThreadSleep(15);
            continue;
        }

        if (SceneInitialized()) {

            s_scene_frame_count += 1;

            if (s_scene_devExecMask & SceneExecMask_GameplayLogic) {
                GameplaySceneLogic(s_world_deltatime.asSeconds());
            }

            if (s_scene_devExecMask & SceneExecMask_HudLogic) {
                //HudSceneLogic();
            }

            dx11_BeginFrameDrawing();

            // Clear background for diagnostic purposes:
            // TODO: Replace this with some pattern that clearly invokes "non-gameplay component", like a gray/black checkerboard.

            dx11_SetRasterState(GPU_Fill_Solid, GPU_Cull_None, GPU_Scissor_Disable);
            dx11_ClearRenderTarget(g_gpu_BackBuffer, GPU_Colors::DarkGray);

            if (s_scene_devExecMask & SceneExecMask_GameplayRender) {
                GameplaySceneRender();
            }
            if (s_scene_devExecMask & SceneExecMask_HudRender) {
                //HudSceneRender();
            }
            DbgFont_SceneRender();
            ImGui::Render();
            dx11_SubmitFrameAndSwap();

            if (Scene_HasStopReason(SceneStopReason_Developer)) {
                EntityManager_CollectGarbage();
            }
        }

        // TODO : framerate pacing (vsync disabled)
        //     Measure time from prev to current frame, determine amount of time we
        //     want to sleep.
        xThreadSleep(16/2);
    }

    s_sem_thread_done.Post();

    return nullptr;
}

void Scene_CreateThreads()
{
    s_scene_thread_running = true;
    thread_create(s_thr_scene_producer, SceneProducerThreadProc, "SceneProducer", _256kb);
}

void Scene_ShutdownThreads()
{
    if (!s_scene_thread_running) return;
    Scene_PostMessage(SceneMsg_Shutdown, 0);
    s_sem_thread_done.WaitWithTimeout(2000);
    s_scene_thread_running = false;
}

bool Scene_HasStopReason(u32 stopReason)
{
    return (s_scene_stopReason & stopReason) != 0;
}
