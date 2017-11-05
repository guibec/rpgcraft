#pragma once
#include "x-simd.h"
#include "x-virtkey.h"

class Mouse
{
	public:

		void		update();
		bool		isPressed(VirtKey_t button) const;

		bool		hasFocus() const { return m_scene_has_focus; }
		bool		isInScene() const { return m_mouse_in_scene; }
		int2		getRelativeToCenterPix() const { return m_mouse_pix_relative_to_center; }
		float2		getRelativeToCenter() const { return m_mouse_pos_relative_to_center; }

	protected:
	private:

	bool	m_scene_has_focus = false;
	bool	m_mouse_in_scene = false;
	int2	m_mouse_pix_relative_to_center = {};
	float2	m_mouse_pos_relative_to_center = {};
};

extern Mouse g_mouse;
