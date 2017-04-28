
#include "x-gpu-ifc.h"


// intentionally empty -- this cpp is only meant to provide a single global instantiation point
// for x-gpu-colors.inl.  On toolchains lacking LTO support, this module should be disabled and
// x-gpu-colors.inl shoudl be included into x-gpu-colors.h directly.

#if TOOLCHAIN_LTO_ENABLED
#	include "x-gpu-colors.inl"
#endif

