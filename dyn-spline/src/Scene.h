#pragma once

#include "x-types.h"
#include "x-stl.h"
#include "x-assertion.h"

#include "ajek-script.h"

static const int SceneMsg_StopExec				= 530;
static const int SceneMsg_StartExec				= 531;
static const int SceneMsg_ToggleExec			= 532;
static const int SceneMsg_StepExec				= 533;

static const int SceneMsg_ReloadScripts			= 100;


static const u32 SceneStopReason_Developer			= 1ul << 0;		// Pause exec via 'P', Stepping exec by frame, etc.
static const u32 SceneStopReason_ScriptError		= 1ul << 1;		// Debugging of script
static const u32 SceneStopReason_Background			= 1ul << 4;		// Game has been placed in the background (console/mobile specific, maybe)
static const u32 SceneStopReason_Dialog				= 1ul << 5;		// Similar to Background but refers to self-made dialogs rather than system0imposed background execution

struct SceneMessage
{
	int		msgid;
	sptr	payload;

	SceneMessage() {}
	SceneMessage( int _msgId, IntPtr _payload )
	{
		msgid	= _msgId;
		payload	= _payload;
	}
};


extern bool			SceneInitialized			();
extern void			SceneInit					();
extern void			SceneRender					();
extern void			SceneBegin					();

extern void			Scene_InitMessages			();
extern void			Scene_ShutdownMessages		();
extern bool			Scene_HasPendingMessages	();
extern bool			Scene_TryLoadInit			(AjekScriptEnv& script);

extern void			Scene_PostMessage			(int msgId, sptr payload);
extern void			Scene_DrainMsgQueue			();
