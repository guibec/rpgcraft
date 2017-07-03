
#include "x-types.h"
#include "x-stl.h"
#include "x-assertion.h"
#include "x-string.h"
#include "x-thread.h"
#include "x-chrono.h"
#include "x-pad.h"
#include "x-host-ifc.h"

#include "Scene.h"
#include "ajek-script.h"

#include <queue>


DECLARE_MODULE_NAME("scene");

typedef std::queue<SceneMessage> SceneMessageList;


static thread_t				s_thr_scene_producer;
static xMutex				s_mtx_MsgQueue;
static SceneMessageList		s_MsgQueue;
static u32					s_scene_isPaused;

static bool					s_scene_initialized			= false;

void Scene_InitMessages()
{
	s_mtx_MsgQueue.Create("SceneMsgQueue");
}

void Scene_ShutdownMessages()
{
	s_mtx_MsgQueue.Delete();
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
				auto& script = AjekScriptEnv_Get(ScriptEnv_Game);
				s_scene_initialized = false;
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

static jmp_buf	s_jmp_buf;

extern void dx11_SetJmpCatch			(jmp_buf& jmpbuf);
extern void dx11_SetJmpFinalize			();
extern void dx11_PrintLastError			();

__ni void SceneInit()
{
	if (s_scene_initialized) {
		return;
	}

	auto& script = AjekScriptEnv_Get(ScriptEnv_Game);

	if (setjmp(s_jmp_buf) == 0) {
		script.SetJmpCatch	(s_jmp_buf);
		dx11_SetJmpCatch	(s_jmp_buf);
		Scene_TryLoadInit(script);
		s_scene_initialized = true;
	}
	else {
		xPrintLn("");
		if (!xIsDebuggerAttached()) {
			log_and_abort("Application aborted due to DoGameInit error."); 
		}
		script.PrintLastError();
		dx11_PrintLastError();
		Scene_PostMessage(SceneMsg_StopExec, SceneStopReason_ScriptError);
		AjekScript_PrintDebugReloadMsg();
	}

	script.SetJmpFinalize	();
	dx11_SetJmpFinalize		();

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
		Scene_DrainMsgQueue();

		if (Scene_HasStopReason(SceneStopReason_ScriptError)) {

		}

		if (Scene_HasStopReason(SceneStopReason_ScriptError | SceneStopReason_Background)) {
			xThreadSleep(32);
			continue;
		}

		if (!SceneInitialized()) {
			SceneInit();
		}

		if (SceneInitialized()) {
			SceneBegin();
			SceneRender();
		}

		// TODO : framerate pacing (vsync disabled)
		//     Measure time from prev to current frame, determine amount of time we
		//     want to sleep.
		xThreadSleep(16/2);
	}

	return nullptr;
}

void Scene_CreateThreads()
{
	thread_create(s_thr_scene_producer, SceneProducerThreadProc, "SceneProducer", _256kb);
}

bool Scene_HasStopReason(u32 stopReason)
{
	return (s_scene_isPaused & stopReason) != 0;
}
