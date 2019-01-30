#pragma once

#include "x-types.h"

// ======================================================================================
//  xCustomAllocator  (interface)
// ======================================================================================
class xCustomAllocator {
public:
    virtual void*       alloc       (size_t size)               =0;
    virtual void*       realloc     (void* ptr, size_t size)    =0;
    virtual void        free        (void* ptr)                 =0;
};

class xDefaultCustomAllocator : public xCustomAllocator {
public:
    void*       alloc       (size_t size)               override;
    void*       realloc     (void* ptr, size_t size)    override;
    void        free        (void* ptr)                 override;
};

extern xDefaultCustomAllocator  g_DefaultCustomAllocator;

extern void     xMalloc_Check       ();
extern void*    xMalloc             (size_t sz);
extern void*    xCalloc             (size_t numItems, size_t sz);
extern void*    xRealloc            (void* srcptr, size_t sz);
extern void     xFree               (void *ptr);
extern void*    xMalloc_Aligned     (size_t sz, u32 align);
extern void*    xRealloc_Aligned    (void* srcptr, size_t sz, u32 align);
extern void     xFree_Aligned       (void* ptr);
extern void     xMalloc_Report      ();
extern void     xMalloc_ReportDelta ();

#define placement_new(T)        new (xMalloc(sizeof(T))) T

// Performs cleanup of existing object pointer (if non-null) and creates a new object in its place.
// Memory is allocated if the provided pointer is null.
template< typename T >
__xi T* xMallocNew(T* &ptr)
{
    if (ptr) {
        ptr->~T();
    }
    else {
        ptr = (T*)xMalloc(sizeof(*ptr));
    }
    return new (ptr) T();
}

// Memory is allocated if the provided pointer is null.  if pointer is non-null, then no action is taken.
template< typename T >
__xi T* xMallocT(T* &ptr)
{
    static_assert(!std::has_virtual_destructor<T>::value, "Non-trivial type has meaningful destructor.  Use xMallocNew<T> instead.");
    //static_assert(!std::is_trivially_copyable<T>::value, "Non-trivial type has meaningful destructor.  Use xMallocNew<T> instead.")
    return ptr ? ptr : xMalloc(sizeof(*ptr));
}
