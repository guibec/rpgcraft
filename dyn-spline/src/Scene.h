#pragma once

#include "x-types.h"
#include "x-stl.h"
#include "x-assertion.h"

#include "ajek-script.h"

// Scene Messaging Remarks:
//   Scene Messages are processed at a rate matching the FPS of the title.  This is between 8ms and 48ms.
//   Anything that must be reliably handled at higher intervals than 48ms must be handled via a different
//   thread.

enum SceneMessageId {
	SceneMsg_Reload					= 100,		// forces reloading of entire scene (scripts, textures, etc)

	SceneMsg_StopExec				= 530,
	SceneMsg_StartExec				= 531,
	SceneMsg_ToggleExec				= 532,
	SceneMsg_StepExec				= 533,
};

// ------------------------------------------------------------------------------------------------
// SceneStopReason
//   These should *always* be specified using Scene_SendMessage API, even when being assigned from
//   the SceneProducer thread.  StopReasons are handled transactionally, such that multiple changes
//   to stop state can be made during a series of synchronous logic operations on SceneThread, and
//   the resultint Stop  State will be calculated once, and coherently.

static const u32 SceneStopReason_Developer			= 1ul << 0;		// Pause exec via 'P', Stepping exec by frame, etc.
static const u32 SceneStopReason_ScriptError		= 1ul << 1;		// Debugging of script
static const u32 SceneStopReason_Background			= 1ul << 4;		// Game has been placed in the background (console/mobile specific, maybe)
static const u32 SceneStopReason_Dialog				= 1ul << 5;		// Similar to Background but refers to self-made dialogs rather than system0imposed background execution
// ------------------------------------------------------------------------------------------------

struct SceneMessage
{
	SceneMessageId		msgid;
	sptr				payload;

	SceneMessage() {}
	SceneMessage(SceneMessageId _msgId, sptr _payload)
	{
		msgid	= _msgId;
		payload	= _payload;
	}
};

extern void			Scene_CreateThreads			();

extern bool			SceneInitialized			();
extern void			SceneInit					();
extern void			SceneRender					();
extern void			SceneBegin					();

extern void			Scene_InitMessages			();
extern void			Scene_ShutdownMessages		();
extern bool			Scene_HasPendingMessages	();
extern bool			Scene_TryLoadInit			(AjekScriptEnv& script);

extern void			Scene_PostMessage			(SceneMessageId msgId, sptr payload);
extern void			Scene_DrainMsgQueue			();
extern bool			Scene_HasStopReason			(u32 stopReason);
