#pragma once

#include "x-types.h"
#include "x-gpu-ifc.h"


struct DevUI_ImageAsset
{
    int2                    size;
    GPU_TextureResource2D   gpures;
};

struct ImGuiTextures
{
    DevUI_ImageAsset        Play;
    DevUI_ImageAsset        Pause;
    DevUI_ImageAsset        Stop;
    DevUI_ImageAsset        Power;

    DevUI_ImageAsset        SoundIcon;
};

extern ImGuiTextures      s_gui_tex;
