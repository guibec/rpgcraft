
#include "PCH-msw.h"
#include "x-chrono.h"

static host_tick_t s_ticks_process_start;

void MSW_InitChrono()
{
	s_ticks_process_start = Host_GetProcessTicks().asTicks();
}

host_tick_t Host_GetProcessTickRate()
{
	LARGE_INTEGER Timebase;
	::QueryPerformanceFrequency( &Timebase );
	return Timebase.QuadPart;
}

HostClockTick Host_GetProcessTicks()
{
	LARGE_INTEGER result;
	QueryPerformanceCounter( &result );
	return HostClockTick(result.QuadPart - s_ticks_process_start);
}

