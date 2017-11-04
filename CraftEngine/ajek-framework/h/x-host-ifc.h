#pragma once

#include "x-types.h"
#include "x-simd.h"
#include "x-virtkey.h"


struct HostMouseState
{
	int2	clientPos;		// position within client relative to upper-left corner
	bool	isValid;		// set FALSE if the mouse info is stale for some reason (determined by host OS).

	struct {
		u8	LBUTTON		: 1;
		u8	RBUTTON		: 1;
		u8	MBUTTON		: 1;
	} pressed;
};

extern void				Host_ImGui_Init				();
extern void				Host_ImGui_NewFrame			();
extern void				_hostImpl_ImGui_NewFrame	();

// --------------------------------------------------------------------------------------
// These functions return the current state of the keyboard, which is ideal for most
// gameplay purposes.  These are not appropriate for use from Keyboard Event message
// queues.

extern bool				Host_IsKeyPressedGlobally	(VirtKey_t key);
extern bool				Host_IsKeyPressedGlobally	(const VirtKeyBindingPair& pair);

extern bool				Host_HasWindowFocus			();
extern VirtKeyModifier	Host_GetKeyModifier			();
extern VirtKeyModifier	Host_GetKeyModifierInMsg	();		// this one OK from windows msg queue
extern bool				HostDiag_IsKeyPressed		(VirtKey_t key);
// --------------------------------------------------------------------------------------

extern void				Host_CaptureMouse			();			// capture included for possible debug usage, but not meant to be used outside of our default wndproc behavior
extern void				Host_ReleaseMouse			();			// capture included for possible debug usage, but not meant to be used outside of our default wndproc behavior
extern HostMouseState	HostMouseImm_GetState		();