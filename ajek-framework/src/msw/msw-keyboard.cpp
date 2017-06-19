
#include "msw-redtape.h"
#include "x-host-ifc.h"

extern HWND g_hWnd;		// must be provided by application's msw-WinMain.cpp

VirtKey_t ConvertFromMswVK( UINT key )
{
	switch (key)
	{
		case VK_ESCAPE:		return VirtKey::Escape;
		case VK_SEPARATOR:	return VirtKey::Separator;
		case VK_OEM_2:		return VirtKey::Slash;				// +shift for Question (US layout)
		case VK_OEM_3:		return VirtKey::BackQuote;			// +shift for tilde	   (US layout)
		case VK_NEXT:		return VirtKey::PageDown;
		case VK_PRIOR:		return VirtKey::PageUp;
		case VK_UP:			return VirtKey::ArrowUp;
		case VK_DOWN:		return VirtKey::ArrowDown;
		case VK_LEFT:		return VirtKey::ArrowLeft;
		case VK_RIGHT:		return VirtKey::ArrowRight;
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

		case VK_LSHIFT:		return VirtKey::Shift;
		case VK_RSHIFT:		return VirtKey::Shift;
		case VK_LCONTROL:	return VirtKey::Ctrl;
		case VK_RCONTROL:	return VirtKey::Ctrl;
		case VK_LMENU:		return VirtKey::Alt;
		case VK_RMENU:		return VirtKey::Alt;
		case VK_LWIN:		return VirtKey::Windows;
		case VK_RWIN:		return VirtKey::Windows;

		default:			return VirtKey::Unmapped;
	};
}

bool Host_IsKeyPressed(VirtKey_t vk_code)
{
	// only grab keyboard input if the focused window is our GS window.
	// ... needed since GetAsyncKeyState returns keystrokes regardless of focus.

	HWND fore = GetForegroundWindow();
	bool weHaveFocus = (g_hWnd == fore || g_hWnd == ::GetParent(fore));;

	if (!weHaveFocus) {
		return 0;
	}

	return (::GetAsyncKeyState(vk_code) & 0x8000u) != 0;
}
