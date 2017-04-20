
#include "mswStandard.h"

#include <process.h>
#include <signal.h>		// for xStopProcess()
#include <cstring>

#include "x-atomic.h"

DECLARE_MODULE_NAME( "msw-thread" );

// needed for thread profiling.
void msw_CloneCurrentThread( thread_t& dest )
{
	dest.msw_thread_id = ::GetCurrentThreadId();
	DuplicateHandle(GetCurrentProcess(),
		GetCurrentThread(),
		GetCurrentProcess(),
		(LPHANDLE)&dest.pseudo_handle,
		0, FALSE, DUPLICATE_SAME_ACCESS
	);
}

// ======================================================================================
//  thread implementation for MSW
// ======================================================================================
// Threads use heap objects internally for condition variables and mutexes, which may not be
// the most optimal solution; but the Olympus project is not reliant on MSW performance either
// and this makes ports to pthreads much simpler.
//

struct ThreadStartStuff
{
	ThreadProc_t*	threadproc;
	void*			data;
	const char*		name;
	xSemaphore		startupSema;
};

static __threadlocal const char* s_thread_name = nullptr;

thread_t thread_self()
{
	thread_t result;
	result.msw_thread_id	= ::GetCurrentThreadId();
	result.pseudo_handle	= -1;
	return result;
}

//
// returned string is only valid for the duration of the thread's life.
//
const char* thread_getname()
{
	return s_thread_name;
}

void thread_setaffinity( u64 threadmask )
{
	DWORD_PTR procmask, sysmask;
	::GetProcessAffinityMask( ::GetCurrentProcess(), &procmask, &sysmask );

	DWORD result = ::SetThreadAffinityMask( ::GetCurrentThread(), procmask & threadmask );
	msw_bug_on_log( !result, "SetThreadAffinityMask failed, mask=%s\nProcess mask=%s", cHexStr(threadmask), cHexStr(procmask) );
}

// --------------------------------------------------------------------------------------
// Must be called from the context of the thread who's name is to be changed.
// Given string must be a valid pointer for the duration of the thread's life.
//
// If the process is attached to a visual studio debugger then the debugger will display
// the assigned name.  Other processes probing the application thread contexts (such as
// Process Explorer) won't see the name, however.
//
void thread_setname( const char* name )
{
	s_thread_name = name;

	if( !::IsDebuggerPresent() ) return;

	//
	// Utilize a not-quite-documented visual studio exception code to set the name
	// of the thread.  Found on an obscure MSDN technet article.
	//

	static const int MS_VC_EXCEPTION = 0x406D1388;

	#pragma pack(push,8)
	struct THREADNAME_INFO
	{
		DWORD dwType;		// Must be 0x1000.
		LPCSTR szName;		// Pointer to name (in user addr space).
		DWORD dwThreadID;	// Thread ID (-1=caller thread).
		DWORD dwFlags;		// Reserved for future use, must be zero.
	};
	#pragma pack(pop)

	THREADNAME_INFO info;
	info.dwType		= 0x1000;
	info.szName		= name;
	info.dwThreadID	= ::GetCurrentThreadId();
	info.dwFlags	= 0;

	__try {
		RaiseException( MS_VC_EXCEPTION, 0, sizeof(info)/sizeof(ULONG_PTR), (ULONG_PTR*)&info );
	} __except(EXCEPTION_EXECUTE_HANDLER) { }
}

static u32 __stdcall ThreadProcDefer_WithData( void* param )
{
	if( bug_on_qa( !param, "NULL parameter passed to created thread.  Aborting thread creation..." ) )
		return 0;

	ThreadStartStuff& stuff = *(ThreadStartStuff*)param;

#if !TARGET_FINAL
	// stack-based copy of thread name; better than using heap alloc since the thread name need only
	// be valid during the context of the thread's lifespan.
	char threadname[32];

	if( stuff.name )
	{
		threadname[0] = 0;
		size_t len = strlen(stuff.name) + 1;
		strncpy_s( threadname, stuff.name, bulkof(threadname) );
		threadname[31] = 0;

		thread_setname( threadname );
	}
#endif

	void*			my_data = stuff.data;
	ThreadProc_t*	my_proc = stuff.threadproc;
	stuff.startupSema.Post();

	void* result = my_proc( my_data );

	s_thread_name = NULL;
	return (u32)result;
}

static u32 __stdcall ThreadProcDefer( void* param )
{
	if( bug_on_qa( !param, "NULL parameter passed to created thread.  Aborting thread creation..." ) )
		return 0;

	ThreadProc_t*	my_proc = (ThreadProc_t*)param;
	void* result = my_proc( NULL );
	return (u32)result;
}

// --------------------------------------------------------------------------------------
static void _thread_create( thread_t& dest, ThreadProc_t* threadproc, const char* name, void* data )
{
	u32 thread_id;
	HANDLE hThread;

	#if TARGET_FINAL
	name = NULL;
	#endif

	if (data || name)
	{
		//
		// Extended (slow) startup procedure.
		// This is needed in order to transfer information to the thread, which is stored
		// on our stackframe.  The thread must digest the info and then signal us that we
		// can continue execution.
		//
		// An alternative is that we allocate data on the heap and then expect the thread
		// to free the handle after it's copied the data onto its stack (or just leave the
		// heap block allocated open until the thread terminates normally).
		//

		ThreadStartStuff stuff;
		stuff.startupSema.Create();
		stuff.threadproc	= threadproc;
		stuff.data			= data;
		stuff.name			= name;

		hThread = (HANDLE)_beginthreadex( nullptr, 0, ThreadProcDefer_WithData, (LPVOID) &stuff, 0, &thread_id );
		stuff.startupSema.Wait();
	}
	else
	{
		hThread = (HANDLE)_beginthreadex( nullptr, 0, ThreadProcDefer, (LPVOID) threadproc, 0, &thread_id );
	}
	
	bug_on_qa( !hThread, "Thread creation failed!" );
	dest.msw_thread_id	= thread_id;
	dest.pseudo_handle	= (sptr)hThread;
}

void thread_create_slim( thread_t& dest, ThreadProc_t* threadproc, const char* name, void* data )
{
	_thread_create(dest, threadproc, name, data);
}

void thread_create( thread_t& dest, ThreadProc_t* threadproc, const char* name, size_t stackSize, void* data )
{
	_thread_create(dest, threadproc, name, data);
}

void thread_exit()
{
	_endthread();
}


// --------------------------------------------------------------------------------------
// Remarks:
//  * This method is tailored for emulator use, and will generate a warning if a join operation
//    takes longer than 1 second.
//  * If longer timeout is desired, then extend this API to take an optional timeout arg.
//
void thread_join( const thread_t& thread )
{
	bug_on_qa (!thread.pseudo_handle,
		"Invalid thread handle state.  thread_t must be used from same thread that created the joinable thread."
	);
	DWORD result = ::WaitForSingleObject( (HANDLE)thread.pseudo_handle, 1200 );
	if (result == WAIT_TIMEOUT)
	{
		log_perf("Join is taking an unexpectedly long time (>1 sec).");
		::WaitForSingleObject( (HANDLE)thread.pseudo_handle, INFINITE );
	}
}

void thread_schedprio( const thread_t& thread, int prio )
{
}

// ======================================================================================
//  mutex implementation for MSW
// ======================================================================================

__eai void mutex_create( mutex_t& dest, const char* name, xMutexType type )
{
	bug_on( dest.obj_handle, "Sure looks like this mutex object is already initialized." );

	CRITICAL_SECTION* obj = new CRITICAL_SECTION;
	dest.obj_handle = obj;
	InitializeCriticalSection( obj );
}

__eai void mutex_delete( mutex_t& dest )
{
	if( !dest.obj_handle ) return;
	CRITICAL_SECTION* obj = (CRITICAL_SECTION*)dest.obj_handle;
	DeleteCriticalSection( obj );
	delete obj;
	dest.obj_handle = NULL;
}

__eai void mutex_lock( mutex_t& mtx )
{
	EnterCriticalSection( (CRITICAL_SECTION*)mtx.obj_handle );
}

__eai void mutex_unlock( mutex_t& mtx )
{
	LeaveCriticalSection( (CRITICAL_SECTION*)mtx.obj_handle );
}

// ======================================================================================
//  semaphore implementation for MSW
// ======================================================================================

__eai void semaphore_create( semaphore_t& dest, const char* name )
{
	dest.obj_handle = (void*)::CreateSemaphoreW( NULL, 0, 0x7fffffff, NULL );
	msw_bug_on_log( !dest.obj_handle );
}

__eai void semaphore_delete( semaphore_t& dest )
{
	::CloseHandle( (HANDLE)dest.obj_handle );
}

__eai void semaphore_post( semaphore_t& sem )
{
	// bizarrely, posting an event to a semaphore is done by calling "ReleaseSemaphore"
	// Terminology that makes sense within the context of a thread pool, but nowhere else
	// one might find themselves using a semaphore. --jstine

	BOOL result = ::ReleaseSemaphore( (HANDLE)sem.obj_handle, 1, nullptr );
	msw_bug_on( !result );
}

__eai void semaphore_wait( semaphore_t& sem )
{
	DWORD result = ::WaitForSingleObject( (HANDLE)sem.obj_handle, INFINITE );
	msw_bug_on(result != WAIT_OBJECT_0);
}

__eai bool semaphore_trywait( semaphore_t& sem )
{
	DWORD result = ::WaitForSingleObject( (HANDLE)sem.obj_handle, 0 );
	if (result == WAIT_TIMEOUT) return false;
	msw_bug_on(result != WAIT_OBJECT_0);
	return true;
}

bool _semaphore_timedwait_impl( semaphore_t& sem, int milliseconds )
{
	// note: Calling function performs paramter verification / assertions.

	DWORD result = ::WaitForSingleObject( (HANDLE)sem.obj_handle, milliseconds );
	if (result == WAIT_TIMEOUT) return false;
	msw_bug_on(result != WAIT_OBJECT_0);
	return true;
}



// ======================================================================================
//  Condition Variable (CondVar) implementation for MSW
// ======================================================================================

__eai void condvar_create( condvar_t& dest )
{
	bug_on( dest.obj_handle, "Sure looks like this condition variable is already initialized." );

	CONDITION_VARIABLE* obj = new CONDITION_VARIABLE;
	dest.obj_handle = obj;
	InitializeConditionVariable( obj );
}

__eai void condvar_delete( condvar_t& dest )
{
	if( !dest.obj_handle ) return;
	CONDITION_VARIABLE* obj = (CONDITION_VARIABLE*)dest.obj_handle;
	// No cleanup needed for condition variables, besides the heap...
	delete obj;
	dest.obj_handle = nullptr;
}

__eai void condvar_signal( condvar_t& cond )
{
	WakeConditionVariable( (CONDITION_VARIABLE*)cond.obj_handle );
}

__eai void condvar_wait( condvar_t& cond, mutex_t& mtx )
{
	SleepConditionVariableCS( (CONDITION_VARIABLE*)cond.obj_handle, (CRITICAL_SECTION*)mtx.obj_handle, INFINITE );
}

// ======================================================================================
//  Barrier  implementation for MSW
// ======================================================================================
// Using an atomic counter and a multi-way semaphore to regulate threads for barrier.
// WaitForMultipleObjects isn't really useful because the thread that ends up completing
// the barrier would have no way of knowing that ahead of time without an atomic counter,
// and once I've bothered to implement the atopic counter, its just as well to use
// semaphores (which are more efficient than WaitForMultipleObjects).
__eai void barrier_create( barrier_t& dest, unsigned int nthr )
{
#if 0
	dest.obj_handles = (void**)xMalloc( sizeof(HANDLE) * nthr );
	bug_on( !dest.obj_handles );

	HANDLE* handles = (HANDLE*)dest.obj_handles;

	for( uint i=0; i<nthr; ++i )
	{
		handles[i] = ::CreateEvent( nullptr, false, false, nullptr );
		msw_bug_on( !handles[i] );
	}
#else
	dest.obj_handle = (void*)::CreateSemaphoreW( nullptr, 0, nthr, nullptr );
	msw_bug_on_log( !dest.obj_handle );
#endif
	dest.nthr			= nthr;
	dest.currentHeight	= nthr;
}

__eai void barrier_delete( barrier_t& dest )
{
#if 0
	if (!dest.obj_handles) return;
	HANDLE* handles = (HANDLE*)dest.obj_handles;
	for( uint i=0; i<dest.nthr; ++i )
	{
		::CloseHandle( handles[i] );
	}
	dest.obj_handles = nullptr;
#else
	::CloseHandle( (HANDLE)dest.obj_handle );
	dest.obj_handle = nullptr;
#endif

	dest.nthr		 = 0;
}

__xi int barrier_wait( barrier_t& dest )
{
	if (0==AtomicDec(dest.currentHeight))
	{
		dest.currentHeight = dest.nthr;
		if (dest.nthr > 1)
		{
			LONG blah;
			BOOL result = ::ReleaseSemaphore( (HANDLE)dest.obj_handle, dest.nthr-1, &blah );
			msw_bug_on( !result );
		}
		return SERIAL_THREAD_VALUE;
	}
	else
	{
		DWORD result = ::WaitForSingleObject( (HANDLE)dest.obj_handle, 0 );
		if (result == WAIT_TIMEOUT) return false;
		msw_bug_on(result != WAIT_OBJECT_0);
		return 0;
	}
}

void xThreadSleep( int millisecs )
{
	Sleep( millisecs );
}

void xThreadYield()
{
	Sleep(0);
}
