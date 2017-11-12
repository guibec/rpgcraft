#pragma once

#include "x-types.h"
#include "x-stl.h"
#include "x-assertion.h"
#include "x-host-ifc.h"

#include "ajek-script.h"
#include "imgui.h"
#include "Entity.h"


// Scene Messaging Remarks:
//   Scene Messages are processed at a rate matching the FPS of the title.  This is between 8ms and 48ms.
//   Anything that must be reliably handled at higher intervals than 48ms must be handled via a different
//   thread.

enum SceneMessageId {
	SceneMsg_Reload					= 100,		// forces reloading of entire scene (scripts, textures, etc)

	SceneMsg_Shutdown				= 200,

	SceneMsg_MouseWheelDelta		= 300,		// ferries mousewheel events from windproc to scene thread (for imgui)
	SceneMsg_KeyDown,
	SceneMsg_KeyUp,
	SceneMsg_KeyChar,

	SceneMsg_StopExec				= 530,
	SceneMsg_StartExec				= 531,
	SceneMsg_ToggleExec				= 532,
	SceneMsg_StepExec				= 533,

};

// SceneStopReason
//   These should *always* be specified using Scene_SendMessage API, even when being assigned from
//   the SceneProducer thread.  StopReasons are handled transactionally, such that multiple changes
//   to stop state can be made during a series of synchronous logic operations on SceneThread, and
//   the resultint Stop State will be calculated once, and coherently.
//
//   GAMEPLAY STOP THINGS DON'T BELONG HERE.  This is related to scene/engine control, and it'll stop
//   everything except ImGui.  Gameplay pauses should be implemented by nullifying the delta-time
//   between frames and/or skipping the Tick step.  (Draw/Render still execute).

static const u32 SceneStopReason_Developer			= 1ul << 0;		// Pause exec via 'P', Stepping exec by frame, etc.
static const u32 SceneStopReason_ScriptError		= 1ul << 1;		// Debugging of script
static const u32 SceneStopReason_Background			= 1ul << 4;		// Game has been placed in the background (console/mobile specific, maybe)
static const u32 SceneStopReason_HostDialog			= 1ul << 5;		// Similar to Background but refers to self-made dialogs rather than system-imposed background execution
static const u32 _SceneStopReason_Shutdown			= 1ul << 7;		// Scene thread is being shut down for good (do not use directly -- invoked by Scene_ShutdownThreads)

// SceneExecMask
//   Used to control which parts of a scene are updated when producing a new frame.  Note that these will
//   also affect any ImGui controls which might also be issued from each step -- eg, entire windows or
//   controls may disappear when specific steps are disabled.
//
//    * If Logic is disabled, the render list from prev frame is preserved and Render will render the
//      previous scene, unmodified.  This may be useful for debugging shaders or other GPU backend operations.
//    * If Render is disabled, then logic will be updated and render lists generated, but no rendering
//      will occur.
//
static const u32 SceneExecMask_GameplayLogic		= 1ul << 0;
static const u32 SceneExecMask_GameplayRender		= 1ul << 1;
static const u32 SceneExecMask_HudLogic				= 1ul << 2;
static const u32 SceneExecMask_HudRender			= 1ul << 3;

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

extern void			Scene_CreateThreads					();
extern void			Scene_ShutdownThreads				();

extern bool			SceneInitialized					();
extern void			SceneInit							();
extern void			GameplaySceneRender					();
extern void			GameplaySceneLogic					(float deltaTime);
extern int			Scene_GetFrameCount					();

extern void			Scene_InitMessages					();
extern bool			Scene_HasPendingMessages			();
extern bool			Scene_TryLoadInit					();

extern void			Scene_PostMessage					(SceneMessageId msgId, sptr payload);
extern void			Scene_DrainMsgQueue					();
extern bool			Scene_HasStopReason					(u32 stopReason = ~0);

extern bool			Scene_IsKeyPressed					(VirtKey_t vk_code);

extern OrderedDrawList		g_drawlist_main;
extern OrderedDrawList		g_drawlist_ui;

namespace
{
	template< int hash_key > bool _impl_OnceUponAFrame() {
		static int RefFrame = -1;
		int current_frame = Scene_GetFrameCount();
		if (RefFrame == current_frame) return false;
		RefFrame = current_frame;
		return true;
	}
};

#define OnceUponAFrame()  _impl_OnceUponAFrame<__COUNTER__>
