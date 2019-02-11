
#include "PCH-rpgcraft.h"

#include "x-types.h"
#include "x-assertion.h"

#include "appConfig.h"

void CliSaveSettingFmt(xString& dest, const char* option_name, _Printf_format_string_ const char* fmt, ...)
{
    if (!cli_bug_chk_option_exists(option_name)) {
        bug_qa("cli save-settings: no parse handler exists for option '%s'", option_name);
    }

    // provides uniform tabulation for entire settings file (no strict requirement to use, purely for cosmetic advantage)
    dest.AppendFmt("%-22s= ", option_name);

    va_list list;
    va_start(list, fmt);
    dest.AppendFmtV(fmt, list);
    dest += '\n';
    va_end(list);
}

void Cli_SaveAudioSettings(xString& dest)
{
    dest.Append   ("\n# Audio Volume Controls\n");
    CliSaveSettingFmt(dest, "audio-global-volume"   , "%3.2f"   , g_settings_audio.glo_volume);
    CliSaveSettingFmt(dest, "audio-bgm-volume"      , "%3.2f"   , g_settings_audio.bgm_volume);
    CliSaveSettingFmt(dest, "audio-sfx-volume"      , "%3.2f"   , g_settings_audio.sfx_volume);
    CliSaveSettingFmt(dest, "audio-nav-volume"      , "%3.2f"   , g_settings_audio.nav_volume);
    CliSaveSettingFmt(dest, "audio-vod-volume"      , "%3.2f"   , g_settings_audio.vod_volume);
    CliSaveSettingFmt(dest, "audio-global-mute"     , "%s"      , g_settings_audio.glo_muted ? "1" : "0");
    CliSaveSettingFmt(dest, "audio-bgm-mute"        , "%s"      , g_settings_audio.bgm_muted ? "1" : "0");
    CliSaveSettingFmt(dest, "audio-sfx-mute"        , "%s"      , g_settings_audio.sfx_muted ? "1" : "0");
    CliSaveSettingFmt(dest, "audio-nav-mute"        , "%s"      , g_settings_audio.nav_muted ? "1" : "0");
    CliSaveSettingFmt(dest, "audio-vod-mute"        , "%s"      , g_settings_audio.vod_muted ? "1" : "0");
}
