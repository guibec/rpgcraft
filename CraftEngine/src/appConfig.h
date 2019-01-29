#pragma once

#include "x-types.h"

// Host Window Settings are only effective (relevant) on host systems where displaying in
// a window is possible (not consoles and not mobile)
struct HostwindowSettings
{
    int2    client_pos;
    int2    client_size;
};

extern HostwindowSettings g_settings_hostwnd;


extern xString      TryFindAsset            (const xString& src);
extern xString      FindAsset               (const xString& src);
extern void         CliParseOption          (const xString& utf8);
extern void         CliParseOptionRaw       (const xString& utf8, const std::function<xString()>& file_line_no, int startpos=0);
extern void         CliParseFromFile        (const xString& fullpath);

extern xString      Host_GetFullPathName    (const xString& relpath);
