#pragma once

#include "x-types.h"

typedef u64 host_tick_t;
typedef s64 tick_delta_t;

struct HostClockTick {

    static host_tick_t  s_ticks_per_second;
    static double       s_ticks_per_second_f;

    static void             Init    ();
    static HostClockTick    Now     ();
    static HostClockTick    Seconds (double secs);

    host_tick_t         m_val;

    HostClockTick() {}

    explicit HostClockTick(u64 src) {
        m_val = src;
    }

    __ai    auto        asTicks             () const    { return m_val; }
    __ai    double      asMicroseconds      () const    { return (double(m_val) * 1000000.0)            / s_ticks_per_second_f; }
    __ai    double      asMilliseconds      () const    { return (double(m_val) *    1000.0)            / s_ticks_per_second_f; }
    __ai    double      asSeconds           () const    { return (double(m_val) *       1.0)            / s_ticks_per_second_f; }
    __ai    double      asMinutes           () const    { return (double(m_val) * (1.0 / (60.0      ))) / s_ticks_per_second_f; }
    __ai    double      asHours             () const    { return (double(m_val) * (1.0 / (60.0*60.0 ))) / s_ticks_per_second_f; }

    __ai bool operator==(const HostClockTick& right) const { return m_val == right.m_val; }
    __ai bool operator!=(const HostClockTick& right) const { return m_val != right.m_val; }

    __ai HostClockTick operator+ (const HostClockTick& right) const { return HostClockTick(m_val + right.m_val); }
    __ai HostClockTick operator- (const HostClockTick& right) const { return HostClockTick(m_val - right.m_val); }

    __ai HostClockTick& operator+= (const HostClockTick& right) { m_val += right.m_val; return *this; }
    __ai HostClockTick& operator-= (const HostClockTick& right) { m_val += right.m_val; return *this; }

    // Delta-time comparisons used to avoid overflow conundruns.

    __ai bool operator> (const HostClockTick& right) const { return tick_delta_t(m_val - right.m_val) >  0; }
    __ai bool operator>=(const HostClockTick& right) const { return tick_delta_t(m_val - right.m_val) >= 0; }
    __ai bool operator< (const HostClockTick& right) const { return tick_delta_t(m_val - right.m_val) <  0; }
    __ai bool operator<=(const HostClockTick& right) const { return tick_delta_t(m_val - right.m_val) <= 0; }

};

// For getting system date information it should be preferable to use std::chrono, which has frameworks
// in place for converting timezones and other fun stuff.  If we have std::chrono overhead issues then
// we can create a worker thread that updates a static var every 16ms or so.

extern HostClockTick    Host_GetProcessTicks        ();
extern host_tick_t      Host_GetProcessTickRate     ();         // intended for internal/init use only, see HostClockTick for cached value


inline HostClockTick HostClockTick::Now() { return Host_GetProcessTicks(); }
inline HostClockTick HostClockTick::Seconds(double secs) {
    return HostClockTick (s_ticks_per_second_f * secs);
}
