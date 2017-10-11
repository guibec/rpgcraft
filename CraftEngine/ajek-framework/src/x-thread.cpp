
#include "PCH-framework.h"
#include "x-thread-internal.h"

bool semaphore_timedwait( semaphore_t& sem, int milliseconds )
{
	// assert ont his because it almost certainly represents bugged logic by the programmer.
	bug_on( milliseconds <= 0,
		"Expected positive/non-zero timeout value, milliseconds=%d\n"
		"For 0ms timeouts, use trywait() instead.", milliseconds
	);

	if (milliseconds <= 0) return true;

	return _semaphore_timedwait_impl( sem, milliseconds );
}

__cxi void xMutex::Create(const xString& name)
{
	_Create(name);
}

__cxi void xSemaphore::Create(const xString& name)
{
	_Create(name);
}

__cxi void xCountedSemaphore::Create(const xString& name)
{
	_Create(name);
}
