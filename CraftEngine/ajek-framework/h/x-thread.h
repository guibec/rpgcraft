
#pragma once

#include "x-assertion.h"
#include "x-stdlib.h"
#include "x-atomic.h"

#define volatize32(x)       (reinterpret_cast<volatile int&>(x))
#define cvolatize32(x)      (reinterpret_cast<volatile const int&>(x))

// --------------------------------------------------------------------------------------
// STRIP_THREAD_NAMES
//
// Set to 1 to strip names from treads and thread objects (mutex, semaphore, etc)
//
#if !defined(STRIP_THREAD_NAMES)
#   define STRIP_THREAD_NAMES                   0
#endif
// --------------------------------------------------------------------------------------

// --------------------------------------------------------------------------------------
// DESTRUCT_THREAD_SYNC_OBJECTS
//
// On PS4 a process is never expected to clean up after itself.  Auto-destruction of mutexes and
// semaphores is not required, and sometimes causes errors when the thread sync object has been
// cleaned up on main thread (during exit()) and is still in use on child threads that haven't yet
// been killed by the system.
//
// Note that the destructor for mutexes and semaphores was always a "precautionary" item and was
// never relied upon for any platform.  Changing DESTRUCT_THREAD_SYNC_OBJECTS=1 should not affect
// application behavior except during at_exit() processing.
//
#if !defined(DESTRUCT_THREAD_SYNC_OBJECTS)
#   define  DESTRUCT_THREAD_SYNC_OBJECTS        0
#endif
// --------------------------------------------------------------------------------------

#if TARGET_ORBIS

typedef ScePthread          thread_t;
typedef ScePthreadMutex     mutex_t;
typedef ScePthreadCond      condvar_t;
typedef SceKernelSema       semaphore_t;
typedef ScePthreadBarrier   barrier_t;

#elif !TARGET_MSW

//
// for linux platforms it's easier to include pthread.h (which is fairly lightweight) and
// base our internal types on it directly.  For msw this doesn't work thanks to windows
// headers being so heavy and intrusive to the global namespace.
//

#   include <pthread.h>
#   include <semaphore.h>

typedef pthread_t           thread_t;
typedef pthread_mutex_t     mutex_t;
typedef pthread_cond_t      condvar_t;
typedef sem_t               semaphore_t;
typedef pthread_barrier_t   barrier_t;

#else

//
// General thread usage warning (MSW):
//   It is really not safe to use blocking-style mutex locks or semaphore waits from the same
//   thread as a Windows Message Pump.  All kinds of important stuff has to go through the
//   message pump (including "under-the-hood" COM mess), so the risk of inadvertent deadlock
//   increases with every mutex or semaphore wait.
//
//   For safe threading, the "host/ui thread" should handle as much as possible via its message
//   queue alone.  If it must communicate with other threads, it should do so via posting
//   messages to a "dependency-free" proxy thread (eg, highly unlikely to deadlock).  The proxy
//   can pass results back to the main/ui thread via the windows message pump.
//

// --------------------------------------------------------------------------------------
//  mutex_t
// --------------------------------------------------------------------------------------
// Implemented as a CRITICAL_SECTION on Windows [implicit recursion support]
// Implemented as a recursive pthread_mutex_t on Linux.
//
struct mutex_t
{
    void*   obj_handle;

    mutex_t()
    {
        obj_handle = NULL;
    }
};

// --------------------------------------------------------------------------------------
//  condvar_t
// --------------------------------------------------------------------------------------
// Implemented as a CONDITION_VARIABLE on Windows
// Implemented as a pthread_cond_t on Linux.
//
struct condvar_t
{
    void*   obj_handle;

    condvar_t()
    {
        obj_handle = NULL;
    }
};

// --------------------------------------------------------------------------------------
//  semaphore_t
// --------------------------------------------------------------------------------------
// Implemented as a HANDLE on Windows [via CreateSemaphore]
// Implemented as a sem_t (pthread semaphore) on Linux.
//
struct semaphore_t
{
    void*   obj_handle;

    semaphore_t()
    {
        obj_handle = NULL;
    }
};

// --------------------------------------------------------------------------------------
//  barrier_t
// --------------------------------------------------------------------------------------
// [TODO] Implement barrier synchronization primitive for Windows.
//
struct barrier_t
{
    int     nthr;
    volatile s32 currentHeight;
    void*   obj_handle;

    barrier_t()
    {
        nthr        = 0;
        obj_handle = nullptr;
    }
};

// --------------------------------------------------------------------------------------
//  thread_t
// --------------------------------------------------------------------------------------
// Implemented as a HANDLE on windows [via CreateThreadEx]
// Implemented as a pthread_t on Linux.
struct thread_t
{
    u32     msw_thread_id;      // globally valid.
    sptr    pseudo_handle;      // valid only from thread that created new thread.

    thread_t()
    {
        msw_thread_id   = 0;
        pseudo_handle   = 0;
    }

    bool operator==( const thread_t& right ) const
    {
        return msw_thread_id == right.msw_thread_id;
    }

    bool operator!=( const thread_t& right ) const
    {
        return msw_thread_id != right.msw_thread_id;
    }
};

#endif

// Note that mutex types are disregarded by MSW (where all mutexes are a
// combination of adaptive + recursive)
enum xMutexType
{
    xMtxType_Default,
    xMtxType_Recursive,
    xMtxType_Adaptive,      // fast spin-lock before thread sleep, ideal for multi-core solutions.
};


typedef volatile u32        atomic_uint_t;
typedef volatile s32        atomic_int_t;

// threadproc signature matches pthreads.
// MSW platforms use a relay function (if needed).
typedef void* ThreadProc_t(void* param);

extern void         thread_create_slim  (thread_t& dest, ThreadProc_t* threadproc, const char* name, void* data=nullptr);
extern void         thread_create       (thread_t& dest, ThreadProc_t* threadproc, const char* name, size_t stackSize=_128kb, void* data=nullptr);
extern thread_t     thread_self         ();
extern void         thread_setname      (const char* name);
extern const char*  thread_getname      ();
extern void         thread_join         (const thread_t& thread);
extern void         thread_schedprio    (const thread_t& thread, int prio);
extern void         thread_setaffinity  (u64 procmask);
extern void         thread_exit         ();

extern void         mutex_create        (mutex_t& dest, const char* name=nullptr, xMutexType type=xMtxType_Default);
extern void         mutex_delete        (mutex_t& dest);

extern void         semaphore_create    (semaphore_t& dest, const char* name=nullptr);
extern void         semaphore_delete    (semaphore_t& dest);
extern bool         semaphore_timedwait (semaphore_t& sem, int milliseconds);

extern void         condvar_create      (condvar_t& dest);
extern void         condvar_delete      (condvar_t& dest);

extern void         barrier_create      (barrier_t& dest, unsigned int nthr);
extern void         barrier_delete      (barrier_t& dest);

#if TARGET_ORBIS
#   define SERIAL_THREAD_VALUE              (SCE_PTHREAD_BARRIER_SERIAL_THREAD)
#elif TARGET_MSW
#   define SERIAL_THREAD_VALUE              (-1)
#else
#   define SERIAL_THREAD_VALUE              (PTHREAD_BARRIER_SERIAL_THREAD)
#endif

#if !TARGET_MSW && !TARGET_ORBIS

//
// Implementations for Linux/BSD PThreads.
//

inline __ai void semaphore_post( semaphore_t& sem )
{
    sem_post(&sem);
}

inline __ai void semaphore_wait( semaphore_t& sem )
{
    sem_wait(&sem);
}

inline __ai bool semaphore_trywait( semaphore_t& sem )
{
    return ( sem_trywait(&sem) == 0 );
}

#if 0
inline __ai int semaphore_getvalue( semaphore_t& sem ) {
    int val = 0;
    sem_getvalue(&sem, &val);
    return val;
}
#endif

inline __ai void mutex_lock( mutex_t& mtx )
{
    pthread_mutex_lock(&mtx);
}

inline __ai void mutex_unlock( mutex_t& mtx )
{
    pthread_mutex_unlock(&mtx);
}

inline __ai void condvar_signal( condvar_t& cond )
{
    pthread_cond_signal(&cond);
}

inline __ai void condvar_wait( condvar_t& cond, mutex_t& mtx )
{
    pthread_cond_wait(&cond, &mtx);
}

inline __ai int barrier_wait( barrier_t& barrier )
{
    return pthread_barrier_wait(&barrier);
}

#else
extern void         semaphore_post      ( semaphore_t& sem );
extern void         semaphore_wait      ( semaphore_t& sem );
extern bool         semaphore_trywait   ( semaphore_t& sem );

extern void         mutex_lock          ( mutex_t& mtx );
extern void         mutex_unlock        ( mutex_t& mtx );

//extern int        semaphore_getvalue  ( semaphore_t& sem );
extern void         condvar_signal      ( condvar_t& cond );
extern void         condvar_wait        ( condvar_t& cond, mutex_t& mtx );
extern int          barrier_wait        ( barrier_t& barrier );

#endif


// --------------------------------------------------------------------------------------
//  xSemaphore  (class)
// --------------------------------------------------------------------------------------
class xSemaphore
{
    NONCOPYABLE_OBJECT( xSemaphore );

protected:
    semaphore_t     m_handle;
    bool            m_is_valid;

protected:
    __ai void _Create(const char* name)
    {
        if (m_is_valid) return;
        m_is_valid = true;
        semaphore_create( m_handle, name );
    }

public:
     void Create        (const xString& name);

public:
    xSemaphore()
    {
        m_is_valid = false;
        //xMemZero( m_handle );
    }

    ~xSemaphore() throw()
    {
        if (DESTRUCT_THREAD_SYNC_OBJECTS) {
            Delete();
        }
    }

    __ai void Create()
    {
        _Create(nullptr);
    }

    __ai void Delete()
    {
        if( !m_is_valid ) return;
        semaphore_delete( m_handle );
        m_is_valid = false;
    }

    __ai void Post()
    {
        semaphore_post( m_handle );
    }

    __ai void Wait()
    {
        semaphore_wait( m_handle );
    }

    __ai bool WaitWithTimeout( int milliseconds )
    {
        return semaphore_timedwait( m_handle, milliseconds );
    }

    __ai bool TryWait()
    {
        return semaphore_trywait( m_handle );
    }
};

// --------------------------------------------------------------------------------------
//  xCountedSemaphore  (class)
// --------------------------------------------------------------------------------------
// A regular semaphore but with a separate counter and GetValue() + PostIfNeeded() methods...
// We need a separate counter because Windows doesn't have a nice way to query the value
// of a semaphore :/
//
// [TODO] this can be replaced by using undocumented function NtQuerySemaphore --jstine
//        http://undocumented.ntinternals.net/UserMode/Undocumented%20Functions/NT%20Objects/Semaphore/NtQuerySemaphore.html
//
class xCountedSemaphore
{
    NONCOPYABLE_OBJECT( xCountedSemaphore );

protected:
    volatile int    m_count;
    semaphore_t     m_handle;

#if DESTRUCT_THREAD_SYNC_OBJECTS
    bool            m_is_valid;
#endif

protected:
    __ai void _Create(const char* name)
    {
    #if DESTRUCT_THREAD_SYNC_OBJECTS
        if (m_is_valid) return;
        m_is_valid = true;
    #endif
        m_count    = 0;
        semaphore_create( m_handle, name );
    }

public:
     void Create        (const xString& name);

public:
    xCountedSemaphore()
    {
    #if DESTRUCT_THREAD_SYNC_OBJECTS
        m_is_valid = false;
    #endif
        m_count    = 0;
        //xMemZero( m_handle );
    }

    ~xCountedSemaphore() throw()
    {
        if (DESTRUCT_THREAD_SYNC_OBJECTS) {
            Delete();
        }
    }

    __ai void Create()
    {
        _Create(nullptr);
    }

    __ai void Delete()
    {
    #if DESTRUCT_THREAD_SYNC_OBJECTS
        if( !m_is_valid ) return;
        m_is_valid = false;
    #endif

        semaphore_delete( m_handle );
        m_count    = 0;
    }

    __ai void Post()
    {
        semaphore_post( m_handle );
        AtomicInc( m_count );
    }

    // Only posts to the semaphore if it doesn't already have
    // any posts queued... Since Post()/Wait() arn't mutex locked,
    // theres is a case where this function will post more than
    // it needs to (e.g. Wait() is being called on another thread
    // but hasn't decremented m_count yet), but over-posting should
    // be fine in typical use-case of xCountedSemaphore...
    __ai void PostIfNeeded()
    {
        if (m_count > 0) return;
        semaphore_post( m_handle );
        AtomicInc( m_count );
    }

    __ai void Wait()
    {
        semaphore_wait( m_handle );
        AtomicDec( m_count );
    }

    __xi bool TryWait()
    {
        bool ret = semaphore_trywait( m_handle );
        if  (ret) AtomicDec( m_count );
        return ret;
    }

    __ai bool WaitWithTimeout( int milliseconds )
    {
        bool result = semaphore_timedwait( m_handle, milliseconds );
        if (result) AtomicDec( m_count );
        return result;
    }

    __ai int GetValue()
    {
        return m_count;
        //return semaphore_getvalue( m_handle );
    }
};

// --------------------------------------------------------------------------------------
//  xMutex
// --------------------------------------------------------------------------------------
// This class is intended for use at either parent class scope or global scope.  Mutexes will
// be cleaned up when the object is deleted, but they will not be unlocked/released (unless
// the underlying OS implementation calls for such behavior).  It is the responsibility of
// the caller to manage mutex lock/unlock status.
//
// See mutex_t for platform-specific implementation notes.
// [TODO] (optimization) -- add support for non-recursive mutexes?  (may be some very small performance
//   gain specific to Linux OS)
//
class xMutex
{
    NONCOPYABLE_OBJECT( xMutex );

protected:
    mutex_t     m_handle;
    xMutexType  m_type;
    bool        m_is_valid;

protected:

    __ai void _Create(const char* name)
    {
        if (m_is_valid) return;
        m_is_valid = true;
        mutex_create( m_handle, name, m_type );
    }

public:
    xMutex()
    {
        m_is_valid      = false;
        m_type          = xMtxType_Default;
    }

    // note that type is disregarded by MSW implementations.
    explicit xMutex( xMutexType type )
    {
        m_is_valid      = false;
        m_type          = type;
    }

#if TARGET_MSW
    // only allowed on windows, which has a fairly lightweight mutex constructor.
    xMutex( bool init_in_place )
    {
        m_is_valid = true;
        mutex_create( m_handle );
    }
#endif

    ~xMutex() throw()
    {
        if (DESTRUCT_THREAD_SYNC_OBJECTS) {
            Delete();
        }
    }

         void           Create      (const xString& name);


    __ai bool           IsValid     () const    { return m_is_valid; }
    __ai void           Lock        ()          { mutex_lock( m_handle );   }
    __ai void           Unlock      ()          { mutex_unlock( m_handle ); }

    __ai operator mutex_t&          ()          { return m_handle; }
    __ai operator const mutex_t&    () const    { return m_handle; }

    __ai void Create()
    {
        _Create(nullptr);
    }

    __ai void Delete()
    {
        if( !m_is_valid ) return;
        mutex_delete( m_handle );
        m_is_valid = false;
    }
};

// --------------------------------------------------------------------------------------
//  xScopedMutex  (class)
// --------------------------------------------------------------------------------------
// For use within neatly encapsulated functions or conditional blocks.  Any code path
// exiting the scope of the block will be sure to unlock the mutex.  The mutex
// passed into the class should be a statically-initialized resource.
//
class xScopedMutex
{
    friend class xCondVar;

protected:
    xMutex*     m_mtx;
    bool        m_isLocked;

public:
    __ai xScopedMutex( xMutex& mtx, bool autoLock = true )
    {
        m_mtx       = &mtx;
        m_isLocked  = autoLock;

        if( autoLock )
        {
            m_mtx->Lock();
            m_isLocked = true;
        }
    }

    __ai ~xScopedMutex() throw()
    {
        Unlock();
    }

    __ai void Lock()
    {
        if( m_isLocked ) return;
        m_mtx->Lock();
        m_isLocked = true;
    }

    __ai void Unlock()
    {
        if( !m_isLocked ) return;
        m_mtx->Unlock();
        m_isLocked = false;
    }
};


// --------------------------------------------------------------------------------------
//  xCondVar
// --------------------------------------------------------------------------------------
class xCondVar
{
    NONCOPYABLE_OBJECT( xCondVar );

protected:
    condvar_t       m_handle;
    bool            m_is_valid;

public:
    xCondVar()
    {
        m_is_valid = false;
        //xMemZero( m_handle );
    }

    ~xCondVar() throw()
    {
        if (DESTRUCT_THREAD_SYNC_OBJECTS) {
            Delete();
        }
    }

    __ai void Create()
    {
        if (m_is_valid) return;
        m_is_valid = true;
        condvar_create( m_handle );
    }

    __ai void Delete()
    {
        if( !m_is_valid ) return;
        condvar_delete( m_handle );
        m_is_valid = false;
    }

    __ai void Signal()
    {
        condvar_signal( m_handle );
    }

    __ai void Wait( xScopedMutex& mtx )
    {
        condvar_wait( m_handle, *mtx.m_mtx );
//      mtx.m_isLocked = false;
    }
};

// --------------------------------------------------------------------------------------
//  xBarrier
// --------------------------------------------------------------------------------------
class xBarrier
{
    NONCOPYABLE_OBJECT( xBarrier );

protected:
    barrier_t       m_handle;

public:
    xBarrier()
    {
    }

    enum {
        BARRIER_SERIAL_THREAD   = SERIAL_THREAD_VALUE,
    };

    ~xBarrier() throw()
    {
        if (DESTRUCT_THREAD_SYNC_OBJECTS) {
            Delete();
        }
    }

    __ai void Create( unsigned int nthr )
    {
        barrier_create( m_handle, nthr );
    }

    __ai void Delete()
    {
        barrier_delete( m_handle );
    }

    __ai int Wait()
    {
        return barrier_wait( m_handle );
    }
};


// --------------------------------------------------------------------------------------
//  xScopedIncremental
// --------------------------------------------------------------------------------------
class xScopedIncremental
{
protected:
    int&        m_dec_me;

public:
    xScopedIncremental( int& dec_me )
        : m_dec_me( dec_me )
    {
    }

    ~xScopedIncremental() throw()
    {
        bug_var(int result = ) AtomicCompareExchange(m_dec_me, 0, 1);
        bug_on( result != 1 );
    }

    bool TryEnter()
    {
        return AtomicCompareExchange(m_dec_me, 1, 0) == 0;
    }
};


extern void     xThreadSleep                ( int milliseconds );
extern void     xThreadYield                ();

extern void     msw_CloneCurrentThread      ( thread_t& dest );
extern void*    msw_GetInstructionPointer   ( thread_t handle );
