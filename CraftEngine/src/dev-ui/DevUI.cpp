#include "PCH-rpgcraft.h"

#include "x-types.h"
#include "x-png-decode.h"
#include "x-chrono.h"

#include "appConfig.h"
#include "Scene.h"
#include "dev-ui/ui-assets.h"


ImGuiTextures      s_gui_tex;


void DevUI_LoadImageAsset(DevUI_ImageAsset& dest, const char* asset_name)
{
    xString fullpath = FindAsset(xFmtStr("dev-ui/%s", asset_name));
    xBitmapData pngsrc;
    png_LoadFromFile(pngsrc, fullpath);
    dx11_CreateTexture2D(dest.gpures, pngsrc, GPU_ResourceFmt_R8G8B8A8_UNORM);
    dest.size = pngsrc.size;
}

void DevUI_LoadStaticAssets()
{
    DevUI_LoadImageAsset(s_gui_tex.Play,      "Play.png");
    DevUI_LoadImageAsset(s_gui_tex.Pause,     "Pause.png");
    DevUI_LoadImageAsset(s_gui_tex.Stop,      "Stop.png");
    DevUI_LoadImageAsset(s_gui_tex.Power,     "Power.png");

    DevUI_LoadImageAsset(s_gui_tex.SoundIcon, "sound-icons.png");
}

enum DevUi_SimplePlayState
{
    SPState_PowerOff = 0,
    SPState_Playing,
    SPState_Paused,
};

static DevUi_SimplePlayState s_spstate = SPState_PowerOff;
static bool s_powerDoubleThrow  = 0;

void DevUI_DevControl()
{
    if (Scene_HasStopReason(SceneStopReason_Developer)) {
        if (s_spstate == SPState_Playing) {
            s_spstate = SPState_Paused;
        }
    }

    if (!ImGui::Begin("DevControl")) return;

    ImVec4 activeBackColor     = { 0.99f, 0.75f, 0.20f, 1.00f };
    ImVec4 activeStateColor    = { 0.00f, 0.20f, 1.00f, 1.00f };
    ImVec4 pwrThrowBackColor   = { 0.22f, 0.22f, 0.22f, 1.00f };
    ImVec4 pwrThrowStateColor  = { 0.99f, 0.10f, 0.10f, 1.00f };
    ImVec4 inactiveBackColor   = { 0,0,0,0 };
    ImVec4 inactiveStateColor  = { 1,1,1,1 };
    ImVec2 buttonSize          = { 24, 24 };

    bool playpress = ImGui::ImageButton((ImTextureID)s_gui_tex.Play.gpures.m_driverData_view, buttonSize, {}, {1,1}, 1,
        (s_spstate == SPState_Playing) ? activeBackColor  : inactiveBackColor,
        (s_spstate == SPState_Playing) ? activeStateColor : inactiveStateColor
    ); ImGui::SameLine();

    bool pausepress = ImGui::ImageButton((ImTextureID)s_gui_tex.Pause.gpures.m_driverData_view, buttonSize, {}, {1,1}, 1,
        (s_spstate == SPState_Paused) ? activeBackColor  : inactiveBackColor,
        (s_spstate == SPState_Paused) ? activeStateColor : inactiveStateColor
    ); ImGui::SameLine();

    bool powerpress = ImGui::ImageButton((ImTextureID)s_gui_tex.Power.gpures.m_driverData_view, buttonSize, {}, {1,1}, s_powerDoubleThrow ? 2 : 1,
        s_powerDoubleThrow ? pwrThrowBackColor  : ((s_spstate == SPState_PowerOff) ? activeBackColor  : inactiveBackColor ),
        s_powerDoubleThrow ? pwrThrowStateColor : ((s_spstate == SPState_PowerOff) ? activeStateColor : inactiveStateColor)
    );

    if (playpress) {
        if (s_spstate != SPState_Playing) {
            //Scene_PostMessage(SceneMsg_StartExec, SceneStopReason_Developer | SceneStopReason_ScriptError);
        }
        s_spstate           = SPState_Playing;
        s_powerDoubleThrow  = 0;
    }

    if (pausepress && (s_spstate == SPState_Playing)) {
        if (s_spstate != SPState_Paused) {
            //Scene_PostMessage(SceneMsg_StopExec, SceneStopReason_Developer);
        }
        s_spstate           = SPState_Paused;
        s_powerDoubleThrow  = 0;
    }

    if (powerpress && (s_spstate != SPState_PowerOff)) {
        if (!s_powerDoubleThrow) {
            s_powerDoubleThrow = 1;
        }
        else {
            // [TODO] send message which wipes game state completely.
            // Subsequent Play action will automatically reset / restart new game state.

            //Scene_PostMessage(SceneMsg_StopExec, SceneStopReason_Developer);
            s_spstate           = SPState_PowerOff;
            s_powerDoubleThrow  = 0;
        }
    }

    ImGui::End();
}
