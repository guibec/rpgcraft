#include "PCH-rpgcraft.h"
#include "Mouse.h"

#include "DbgFont.h"
#include "Scene.h"

static HostMouseState	s_mouseState;

bool Mouse::isPressed(VirtKey_t button) const
{
	return isClickable() && Host_IsKeyPressedGlobally(button);
}

// returns true if the mouse is in a trackable state, for position tracking only.
// Do not use this for button press response logic, since button presses must adhere to UI
// overlay rules!
bool Mouse::isTrackable() const
{
	// any valid mouse data is considered trackable by default, whether it's overtop a
	// UI element or not.
	return s_mouseState.isValid;
}

bool Mouse::isClickable() const
{
	return s_mouseState.isValid && !m_obstructed_by_ui && m_normalized.isInsideArea;
}

// allows for off-center normalization of backbuffer sub-area
MouseNormalResult Mouse::clientToNormal(const int2& center_pix, const int4& viewarea) const
{
	auto viewsize   = int2 { (viewarea.u - viewarea.x), (viewarea.v - viewarea.y) };
	auto ratio		= float(viewsize.x) / float(viewsize.y);
	auto relpos		= s_mouseState.clientPos - (viewarea.xy + center_pix);
	auto normalized = (float2(relpos) / viewsize) * 2.0f;

	MouseNormalResult result;
	result.normal		 = normalized;
	result.normal.x		*= ratio;
	result.isInsideArea  = (fabsf(normalized) <= 1.0f);

	return result;
}

// view size is cenered around 'center_pix' area.  For use in calculating mouse position
// within a render target view which has been projected onto the screen.
MouseNormalResult Mouse::clientToNormal(const int2& center_pix, const int2& viewsize_pix) const
{
	auto ratio		= float(viewsize_pix.x) / float(viewsize_pix.y);
	auto relpos		= s_mouseState.clientPos - center_pix;
	auto normalized = (float2(relpos) / viewsize_pix) * 2.0f;

	MouseNormalResult result;
	result.normal		 = normalized;
	result.normal.x		*= ratio;
	result.isInsideArea  = (fabsf(normalized) <= 1.0f);

	return result;
}

MouseNormalResult Mouse::clientToNormal(const int2& center_pix) const
{
	return clientToNormal(center_pix, g_client_size_pix);
}

MouseNormalResult Mouse::clientToNormal() const
{
	return m_normalized;
}

void Mouse::update()
{
	m_obstructed_by_ui = ImGui::GetIO().WantCaptureMouse;
	s_mouseState = HostMouseImm_GetState();
	m_normalized = {};
	if (s_mouseState.isValid) {
		m_normalized = clientToNormal(g_client_size_pix/2, g_client_size_pix);
	}
}