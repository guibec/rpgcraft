#pragma once

#include "x-types.h"

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
		Unmapped = 0,
		Escape   = 0x1000,
		Slash,
		Separator,		// backslash or pipe (\|)
		BackQuote,
		PageDown,
		PageUp,
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

		Shift,
		Ctrl,
		Alt,
		Windows,
	};
};

typedef u32 VirtKey_t;

extern bool			Host_IsKeyPressed	(VirtKey_t key);
