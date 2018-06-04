
//
// INITGUID  (why?)
//
// This is needed for DirectInput8 used by msw-pad.cpp.  This placement is ideal given the following conditions:
//   1. Visual Studio 2010 or newer
//   2. Precompiled headers enabled
// If either condition is not met, you'll probably get multiple definition errors at the linker stage, and will
// have to move this define to some other location (not sure where, don't care to find out ). --jstine
// 

#ifdef _WIN32
#	define INITGUID
#endif

#include "mswStandard.h"

