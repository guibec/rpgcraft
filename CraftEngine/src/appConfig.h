#pragma once

#include "x-types.h"

// Host Window Settings are only effective (relevant) on host systems where displaying in
// a window is possible (not consoles and not mobile)
//  - use HostwindowSettings().client_size to get the original defalut value.
struct HostwindowSettings
{
    int2    client_pos              = {   64,   64 };
    int2    client_size             = { 1280,  720 };
    bool    has_client_pos          = false;
    bool    has_client_size         = false;
};

struct HostAppSettings
{
    int2    backbuffer_size         = { 1280,  720 };
    bool    has_backbuffer_size     = false;
    bool    windowless_mode         = false;
    int     kill_at_frame_number    = 0;
};

struct AudioSettings
{
    // global sound control.
    // [FIXME] Unimplemented!  is it better to use an FMOD channel group to implement this?
    //         Or add some functions to this struct that apply the multipler ourselves?
    //         FMOd's channel groupings are probably over-engineering for the simple task at hand.
    //         If we only ever use one channel group, why bother with a full opaque API for it ?  --jstine

    float      glo_volume = 1.0f;
    bool       glo_muted  = 0;

    // background music
    float      bgm_volume = 1.0f;
    bool       bgm_muted  = 0;

    // in-game sound effects
    float      sfx_volume = 1.0f;
    bool       sfx_muted  = 0;

    // nav / menu sound effects
    // (controls for these are provided for developer balance/tweaking and probably
    // won't be exposed tot he end user)
    float      nav_volume = 1.0f;
    bool       nav_muted  = 0;

    // vod - voice-over dialogs
    float      vod_volume = 1.0f;
    bool       vod_muted  = 0;
};


extern HostAppSettings      g_settings_app;
extern HostwindowSettings   g_settings_hostwnd;
extern AudioSettings        g_settings_audio;

extern void         MarkUserSettingsDirty   ();

extern xString      TryFindAsset            (const xString& src);
extern xString      FindAsset               (const xString& src);
extern void         CliParseOption          (const xString& utf8);
extern void         CliParseOptionRaw       (const xString& utf8, const std::function<xString()>& file_line_no, int startpos=0);
extern void         CliParseFromFile        (const xString& srcfullpath);
extern void         CliSaveSettingFmt       (xString& dest, const char* option_name, _Printf_format_string_ const char* fmt, ...) __verify_fmt(3, 4);

extern xString      Host_GetFullPathName    (const xString& relpath);


extern void         Cli_SaveDesktopSettings   (xString& dest);
extern void         Cli_SaveAudioSettings     (xString& dest);

extern bool         cli_bug_chk_option_exists       (const char* option);
extern void         cli_bug_chk_option_complete     ();

