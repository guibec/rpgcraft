
#include "msw-redtape.h"
#include "x-host-ifc.h"

DECLARE_MODULE_NAME("x-kbd");

extern HWND g_hWnd;		// must be provided by application's msw-WinMain.cpp

VirtKey_t ConvertFromMswVK( UINT key )
{
	switch (key)
	{
		case VK_ESCAPE:		return VirtKey::Escape;
		case VK_RETURN:		return VirtKey::Enter;
		case VK_SEPARATOR:	return VirtKey::Separator;
		case VK_OEM_2:		return VirtKey::Slash;				// +shift for Question (US layout)
		case VK_OEM_3:		return VirtKey::BackQuote;			// +shift for tilde	   (US layout)
		case VK_NEXT:		return VirtKey::PageDown;
		case VK_PRIOR:		return VirtKey::PageUp;
		case VK_DELETE:		return VirtKey::Delete;
		case VK_HOME:		return VirtKey::Home;
		case VK_END:		return VirtKey::End;
		case VK_UP:			return VirtKey::ArrowUp;
		case VK_DOWN:		return VirtKey::ArrowDown;
		case VK_LEFT:		return VirtKey::ArrowLeft;
		case VK_RIGHT:		return VirtKey::ArrowRight;

		case VK_NUMPAD1:	return VirtKey::KeyPad_1;
		case VK_NUMPAD2:	return VirtKey::KeyPad_2;
		case VK_NUMPAD3:	return VirtKey::KeyPad_3;
		case VK_NUMPAD4:	return VirtKey::KeyPad_4;
		case VK_NUMPAD5:	return VirtKey::KeyPad_5;
		case VK_NUMPAD6:	return VirtKey::KeyPad_6;
		case VK_NUMPAD7:	return VirtKey::KeyPad_7;
		case VK_NUMPAD8:	return VirtKey::KeyPad_8;
		case VK_NUMPAD9:	return VirtKey::KeyPad_9;
		case VK_NUMPAD0:	return VirtKey::KeyPad_0;

		case VK_F1:			return VirtKey::F1;
		case VK_F2:			return VirtKey::F2;
		case VK_F3:			return VirtKey::F3;
		case VK_F4:			return VirtKey::F4;
		case VK_F5:			return VirtKey::F5;
		case VK_F6:			return VirtKey::F6;
		case VK_F7:			return VirtKey::F7;
		case VK_F8:			return VirtKey::F8;
		case VK_F9:			return VirtKey::F9;
		case VK_F10:		return VirtKey::F10;
		case VK_F11:		return VirtKey::F11;
		case VK_F12:		return VirtKey::F12;

		case VK_LSHIFT:		return VirtKey::LShift;
		case VK_RSHIFT:		return VirtKey::RShift;
		case VK_LCONTROL:	return VirtKey::LCtrl;
		case VK_RCONTROL:	return VirtKey::RCtrl;
		case VK_LMENU:		return VirtKey::LAlt;
		case VK_RMENU:		return VirtKey::RAlt;
		case VK_LWIN:		return VirtKey::LWin;
		case VK_RWIN:		return VirtKey::RWin;

		default:			return VirtKey::Unmapped;
	};
}

bool Host_HasWindowFocus()
{
	HWND fore = GetForegroundWindow();
	return (g_hWnd == fore || g_hWnd == ::GetParent(fore));;
}

// Bypasses window focus checks -- intended for use in development, eg. to pick up specific
// key strokes while developer has another editor window active.
bool Host_IsKeyPressedGlobally(VirtKey_t vk_code)
{
	if (vk_code == VirtKey::Unmapped) return false;
	return (::GetAsyncKeyState(vk_code) & 0x8000u) != 0;
}

bool Host_IsKeyPressedGlobally(const VirtKeyBindingPair& pair)
{
	return Host_IsKeyPressedGlobally(pair.primary) || Host_IsKeyPressedGlobally(pair.secondary);
}


// Does full window focus checks on-the-stop.  Not intended for use in main gameplay
// logic loops due to potential overhead of querying window focus from OS.
bool HostDiag_IsKeyPressed(VirtKey_t vk_code)
{
	// only grab keyboard input if the focused window is our GS window.
	// ... needed since GetAsyncKeyState returns keystrokes regardless of focus.

	if (!Host_HasWindowFocus()) {
		return 0;
	}

	return (::GetAsyncKeyState(vk_code) & 0x8000u) != 0;
}

VirtKeyModifier Host_GetKeyModifier()
{
	VirtKeyModifier result;

	//log_host( "JOY TO THE WORLD: %04x", GetAsyncKeyState(VK_CONTROL) & 0x8000);

	result.ctrl		=	!!(::GetAsyncKeyState(VK_CONTROL	) & 0x8000);
	result.shift	=	!!(::GetAsyncKeyState(VK_SHIFT		) & 0x8000);
	result.alt		=	!!(::GetAsyncKeyState(VK_MENU		) & 0x8000);
	result.windows	=	!!(::GetAsyncKeyState(VK_LWIN		) & 0x8000);
					  //||(::GetAsyncKeyState(VK_RWIN		) & 0x8000);

	// Hmm... the RWIN check is untested:
	//    most keyboards don't have an LWIN and microsoft doesn't provide a unified WIN key reader.
	//    Microsoft provides RWIN on it's Naturla keyboard (maybe?).  That's what the MSDN says, but
	//    can't really trust it.  LWIN might just be legacy at this point.

	return result;
}

