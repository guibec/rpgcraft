
#include "x-types.h"
#include "x-stl.h"
#include "x-assertion.h"
#include "x-string.h"
#include "x-thread.h"

#include "Scene.h"
#include "ajek-script.h"


typedef std::list<SceneMessage> SceneMessageList;


xMutex				s_mtx_MsgQueue;
SceneMessageList	s_MsgQueue;
u32					s_scene_isPaused;

void Scene_InitMessages()
{
	s_mtx_MsgQueue.Create("EmuMsgQueueVM");
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

void Scene_PostMessage( int msgId, IntPtr payload )
{
	xScopedMutex lock(s_mtx_MsgQueue);
	s_MsgQueue.push_back( SceneMessage(msgId, payload) );
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
		s_MsgQueue.pop_front();
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
		}
	}
}

static bool s_scene_initialized			= false;

void Scene_ForceUpdateScripts()
{
	auto& script = AjekScriptEnv_Get(ScriptEnv_Game);

	if (script.HasError()) {
		s_scene_initialized = false;
	}
}

__ai bool SceneInitialized() {
	return s_scene_initialized;
}

__ni void SceneInit()
{
	if (s_scene_initialized) {
		return;
	}

	auto& script = AjekScriptEnv_Get(ScriptEnv_Game);

	if (AjekScript_SetJmpIsOK(script)) {
		Scene_TryLoadInit(script);
		s_scene_initialized = true;
	}
	else {
		if (!xIsDebuggerAttached()) {
			log_and_abort("Application aborted due to DoGameInit error."); 
		}
		AjekScript_PrintDebugReloadMsg();
		script.PrintLastError();
	}
}

