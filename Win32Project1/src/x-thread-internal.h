
#pragma once

// This file should be in common/src/ !  It is meant for internal use by the common libraries only.
//  (stuff in common/h/ is exposed to external libs)


#include "x-thread.h"

extern bool _semaphore_timedwait_impl( semaphore_t& sem, int milliseconds );
