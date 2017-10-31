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

#include "appConfig.h"
#include "ajek-script.h"

#include "Scene.h"
#include "Entity.h"
#include "DbgFont.h"

#include <queue>


DECLARE_MODULE_NAME("scene");

typedef std::queue<SceneMessage> SceneMessageList;

static thread_t				s_thr_scene_producer;
static xMutex				s_mtx_MsgQueue;
static xSemaphore			s_sem_thread_done;
static SceneMessageList		s_MsgQueue;
static u32					s_scene_isPaused;

static bool					s_scene_initialized			= false;
static bool					s_scene_thread_running		= false;

static int					s_scene_frame_count			= 0;


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
	bool processed	= false;		// flag if any msgs are processed at all.

	xScopedMutex lock(s_mtx_MsgQueue);
	while (!s_MsgQueue.empty())
	{
		SceneMessage msg = s_MsgQueue.front();
		s_MsgQueue.pop();
		lock.Unlock();
		processed = true;

		switch(msg.msgid)
		{
			case SceneMsg_StopExec: {
				s_scene_isPaused	|=  _getStopReason(msg.payload);
			} break;

			case SceneMsg_StartExec: {
				s_scene_isPaused	&= ~_getStopReason(msg.payload);
			} break;

			case SceneMsg_ToggleExec: {
				s_scene_isPaused    ^=  _getStopReason(msg.payload);
			} break;

			case SceneMsg_StepExec: {
				s_scene_isPaused	^= SceneStopReason_Developer;
				//step_exec			 = !s_scene_isPaused;
			} break;

			case SceneMsg_Reload: {
				s_scene_initialized = false;
			} break;

			case SceneMsg_Shutdown: {
				s_scene_isPaused |= _SceneStopReason_Shutdown;
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

		DbgFont_LoadInit	();
		Scene_TryLoadInit	();
		s_scene_initialized = true;
	}
	x_catch() {
		case xThrowModule_GPU:
		case xThrowModule_Script:
		case xThrowModule_Bridge:
			xPrintLn("");
			g_ThrowCtx.PrintLastError();

			if (!xIsDebuggerAttached()) {
				log_and_abort("Application aborted during SceneInit.");
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

	auto modifiers	= Host_GetKeyModifier();

	// Repeater detection should be bound to the key and not to modifiers.
	// therefore always check key FIRST, and then modifiers within...

	if (IsKeyPressedGlobally('R')) {
		if (modifiers.CtrlWin()) {
			// todo.
		}
	}
}

static void* SceneProducerThreadProc(void*)
{

	while(1)
	{
		Host_ImGui_NewFrame();
		DbgFont_NewFrame();
		Scene_DrainMsgQueue();

		if (Scene_HasStopReason(_SceneStopReason_Shutdown)) {
			log_host("SceneThread has been shutdown.");
			break;
		}

		if (Scene_HasStopReason(SceneStopReason_ScriptError | SceneStopReason_Developer)) {
			dx11_BeginFrameDrawing();
			DbgFont_SceneRender();
			ImGui::Render();
			dx11_SubmitFrameAndSwap();
			xThreadSleep(32);
			continue;
		}

		if (Scene_HasStopReason(SceneStopReason_Background | SceneStopReason_HostDialog)) {
			xThreadSleep(32);
			continue;
		}

		if (!SceneInitialized()) {
			SceneInit();
		}

		if (SceneInitialized()) {
			s_scene_frame_count += 1;
			SceneLogic();

			dx11_BeginFrameDrawing();
			SceneRender();
			DbgFont_SceneRender();
			ImGui::Render();
			dx11_SubmitFrameAndSwap();
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
	return (s_scene_isPaused & stopReason) != 0;
}
