#pragma once

#include "x-types.h"

typedef u64 host_tick_t;

struct HostClockTick {

	static host_tick_t	s_ticks_per_second;
	static float		s_ticks_per_second_f;

	static void			Init();

	host_tick_t			m_val;

	HostClockTick() {}

	explicit HostClockTick(u64 src) {
		m_val = src;
	}

	__ai	auto		asTicks				() const	{ return m_val; }
	__ai	float		asMicroseconds		() const	{ return (m_val * 1000000.0f)				/ s_ticks_per_second_f;	}
	__ai	float		asMilliseconds		() const	{ return (m_val *    1000.0f)				/ s_ticks_per_second_f;	}
	__ai	float		asSeconds			() const	{ return (m_val *       1.0f)				/ s_ticks_per_second_f;	}
	__ai	float		asMinutes			() const	{ return (m_val * (1.0f / (60.0f      )))	/ s_ticks_per_second_f;	}
	__ai	float		asHours				() const	{ return (m_val * (1.0f / (60.0f*60.0f)))	/ s_ticks_per_second_f;	}
};

// For getting system date information it should be preferable to use std::chrono, which has frameworks
// in place for converting timezones and other fun stuff.  If we have std::chrono overhead issues then
// we can create a worker thread that updates a static var every 16ms or so.

extern HostClockTick	Host_GetProcessTicks		();
extern host_tick_t		Host_GetProcessTickRate		();			// intended for internal/init use only, see HostClockTick for cached value

