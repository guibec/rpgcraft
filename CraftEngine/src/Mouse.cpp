#include "PCH-rpgcraft.h"
#include "Mouse.h"

#include "DbgFont.h"
#include "Scene.h"

#include "imgui_impl_dx11.h"

void Mouse::update()
{   
    m_mouse_in_scene = false;
	m_scene_has_focus = !ImGui::GetIO().WantCaptureKeyboard && Host_HasWindowFocus();

	if (!ImGui::GetIO().WantCaptureMouse) {
		HostMouseImm_UpdatePoll();
		if (HostMouseImm_HasValidPos()) {
			int2 relpos = HostMouseImm_GetClientPos() - (g_backbuffer_size_pix/2);

			auto ratio		= float(g_backbuffer_size_pix.x) / float(g_backbuffer_size_pix.y);
			auto normalized = ((float2)relpos / (float2)(g_backbuffer_size_pix)) * 2.0f;

			m_mouse_in_scene = (fabsf(normalized) <= 1.0f);

			normalized.x   *= ratio;
			g_DbgFontOverlay.Write(0,4, xFmtStr("Mouse: %4d %4d  client=%s", relpos.x, relpos.y, m_mouse_in_scene ? "yes" : "no" ));
			g_DbgFontOverlay.Write(0,5, xFmtStr("Norm : %5.3f %5.3f", normalized.x, normalized.y));
			m_mouse_pos_relative_to_center = normalized;
		}
		else {
			m_mouse_in_scene = false;
			m_mouse_pos_relative_to_center = {};
		}
	}
}