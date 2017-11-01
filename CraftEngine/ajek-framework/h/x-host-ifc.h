#pragma once

#include "x-types.h"
#include "x-simd.h"

// --------------------------------------------------------------------------------------
// Cross-platform Virtual key map.
// Alphanumeric characters and punctuation are assumed to be supported via their lowercase form:
//    'a','b','c','1','2','3','-','=', etc.
//
// Provided as namespace to allow VirtKey::* usage which, for this particular type, makes sense.
//
namespace VirtKey
{
	enum _enum_t {
		Unmapped		= 0,

		MouseLeft		= 0x10,
		MouseRight,
		MouseMiddle,

		Escape			= 0x1000,
		Enter,
		Tab,
		Slash,
		BackQuote,
		Separator,		// backslash or pipe (\|)  [not available on JP keyboards]

		PageDown,
		PageUp,
		Delete,
		Home,
		End,
		ArrowUp,
		ArrowLeft,
		ArrowDown,
		ArrowRight,


		KeyPad_1,
		KeyPad_2,
		KeyPad_3,
		KeyPad_4,
		KeyPad_5,
		KeyPad_6,
		KeyPad_7,
		KeyPad_8,
		KeyPad_9,
		KeyPad_0,

		F1,
		F2,
		F3,
		F4,
		F5,
		F6,
		F7,
		F8,
		F9,
		F10,
		F11,
		F12,

		LShift,
		LCtrl,
		LAlt,
		LWin,

		RShift,
		RCtrl,
		RAlt,
		RWin,


	};
};

typedef u32 VirtKey_t;

struct VirtKeyBindingPair {
	VirtKey_t		primary;
	VirtKey_t		secondary;
};

// --------------------------------------------------------------------------------------
//  VirtKeyModifier
// --------------------------------------------------------------------------------------
// This structure has been intentionally designed to avoid considering Left and Right differentiation
// of modifier keys.  Keeping track fo such logic complicates the struct considerably and makes
// methods of the struct behave non-intuitively.  If differentiation of left and right modifiers
// is required, then you must poll the keyboard state directly using Host_IsKeyPressed().
//
struct VirtKeyModifier
{
	static const u32 MSK_CTRL		= (1ULL<<0);
	static const u32 MSK_SHIFT		= (1ULL<<1);
	static const u32 MSK_ALT		= (1ULL<<2);
	static const u32 MSK_WIN		= (1ULL<<3);

	union {
		struct {
			u8		ctrl			: 1;
			u8		shift			: 1;
			u8		alt				: 1;
			u8		windows			: 1;
		};

		u8		b;
	};

	VirtKeyModifier		(u32 src=0)			{ b = src; }

	bool Is				(u32 mask)	const	{ return b == mask;			}

	bool None			()			const	{ return !b;				}
	bool Ctrl			()			const	{ return Is(MSK_CTRL	);	}
	bool Shift			()			const	{ return Is(MSK_SHIFT	);	}
	bool Alt			()			const	{ return Is(MSK_ALT		);	}
	bool Win			()			const	{ return Is(MSK_WIN		);	}

	bool CtrlShift		()			const	{ return Is(MSK_CTRL	| MSK_SHIFT);	}
	bool CtrlWin		()			const	{ return Is(MSK_CTRL	| MSK_WIN	);	}
	bool ShiftWin		()			const	{ return Is(MSK_SHIFT	| MSK_WIN	);	}
	bool CtrlAlt		()			const	{ return Is(MSK_CTRL	| MSK_ALT	);	}
	bool ShiftAlt		()			const	{ return Is(MSK_SHIFT	| MSK_ALT	);	}
};

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

extern void				Host_ImGui_NewFrame			();

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