#include "PCH-rpgcraft.h"

#include "x-types.h"

#include "Scene.h"


    static bool s_isPlaying = true;
    static bool s_isPaused  = false;

void DevUI_DevControl()
{
        if (Scene_HasStopReason(SceneStopReason_Developer)) {
            s_isPaused = 1;
        }

        if (ImGui::Begin("DevControl")) {
            ImVec4 activeBackColor     = { 0.99f, 0.75f, 0.20f, 1.00f };
            ImVec4 activeStateColor    = { 0.00f, 0.20f, 1.00f, 1.00f };
            ImVec4 inactiveBackColor   = { 0,0,0,0 };
            ImVec4 inactiveStateColor  = { 1,1,1,1 };
            ImVec2 buttonSize          = { 24, 24 };

            bool playpress = ImGui::ImageButton((ImTextureID)s_gui_tex.Play.gpures.m_driverData_view, buttonSize, {}, {1,1}, 1,
                s_isPlaying && !s_isPaused ? activeBackColor  : inactiveBackColor,
                s_isPlaying && !s_isPaused ? activeStateColor : inactiveStateColor
            ); ImGui::SameLine();

            bool pausepress = ImGui::ImageButton((ImTextureID)s_gui_tex.Pause.gpures.m_driverData_view, buttonSize, {}, {1,1}, 1,
                s_isPaused ? activeBackColor  : inactiveBackColor,
                s_isPaused ? activeStateColor : inactiveStateColor
            ); ImGui::SameLine();

            bool powerpress = ImGui::ImageButton((ImTextureID)s_gui_tex.Power.gpures.m_driverData_view, buttonSize, {}, {1,1}, 1,
                !s_isPlaying ? activeBackColor  : inactiveBackColor,
                !s_isPlaying ? activeStateColor : inactiveStateColor
            );

            if (playpress) {
                s_isPaused  = 0;
                if (!s_isPlaying) {
                    s_isPlaying = 1;
                }
            }
            if (pausepress) {
                s_isPaused  = 1;
            }
            if (powerpress) {
                if (s_isPlaying) {
                }
                s_isPlaying = 0;
                s_isPaused  = 0;
            }
        } ImGui::End();
}
