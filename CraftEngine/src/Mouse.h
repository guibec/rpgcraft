#pragma once
#include "x-simd.h"
#include "x-virtkey.h"

struct MouseNormalResult
{
    float2  normal;             // normal adjusted by display ratio
    bool    isInsideArea;       // use for click filtering, but not for tracking filtering
};

class Mouse
{
    public:

        void                update();
        bool                isPressed(VirtKey_t button) const;

        bool                isTrackable() const;
        bool                isClickable() const;
        MouseNormalResult   clientToNormal() const;
        MouseNormalResult   clientToNormal(const int2& center_pix) const;                               // center is relative to backbuffer (client) area
        MouseNormalResult   clientToNormal(const int2& center_pix, const int2& viewsize_pix) const;     // center is relative to backbuffer (client) area
        MouseNormalResult   clientToNormal(const int2& center_pix, const int4& viewarea) const;         // center is relative to backbuffer (client) area

    public:
        MouseNormalResult m_normalized;
        bool    m_obstructed_by_ui  = false;
};

extern Mouse g_mouse;
