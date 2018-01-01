
#pragma once

#include "x-stdlib.h"

// --------------------------------------------------------------------------------------
//  BaseScopedMalloc
// --------------------------------------------------------------------------------------
// Simple strongly-typed self-destructing container for malloc().
//
template< typename T >
class BaseScopedMalloc
{
protected:
    T*      m_buffer;
    uint    m_size;

public:
    BaseScopedMalloc()
    {
        m_buffer    = nullptr;
        m_size      = 0;
    }

    virtual ~BaseScopedMalloc() throw() { }

public:
    size_t      GetSizeInBytes  () const { return m_size * sizeof(T); }
    size_t      GetCount        () const { return m_size; }
    bool        IsValid         () const { return !!m_buffer; }

    // Destroys existing allocation and replaces it with a new allocation (data loss)
    virtual void Reset      ( size_t size=0 )=0;

    // Reallocates existing allocation.  No data loss if newsize is larger, and truncated
    // data loss if newsize is smaller.
    virtual void Resize     ( size_t newsize )=0;

    void Free()
    {
        Reset();
    }
    
    T* GetPtr( size_t idx=0 ) const
    {
        #if SECURE_SCOPED_MALLOC
        bug_on( idx >= m_size, "Index out of bounds.  idx=%s, size=%s", cDecStr(idx), cDecStr(m_size) );
        #endif
        return &m_buffer[idx];
    }

    T& operator[]( uint idx )
    {
        #if SECURE_SCOPED_MALLOC
        bug_on( idx >= m_size, "Index out of bounds.  idx=%s, size=%s", cDecStr(idx), cDecStr(m_size) );
        #endif
        return m_buffer[idx];
    }

    const T& operator[]( uint idx ) const
    {
        #if SECURE_SCOPED_MALLOC
        bug_on( idx >= m_size, "Index out of bounds.  idx=%s, size=%s", cDecStr(idx), cDecStr(m_size) );
        #endif
        return m_buffer[idx];
    }
};

// --------------------------------------------------------------------------------------
//  ScopedMalloc  (class)
// --------------------------------------------------------------------------------------
// A simple container class for a standard malloc allocation.  By default, no bounds checking
// is performed, and there is no option for enabling bounds checking.  If bounds checking and
// other features are needed, use the more robust SafeArray<> instead.
//
// See docs for BaseScopedAlloc for details and rationale.
//
template< typename T >
class ScopedMalloc : public BaseScopedMalloc<T>
{
    typedef BaseScopedMalloc<T> __parent;

protected:
    using __parent::m_buffer;
    using __parent::m_size;

public:
    using __parent::operator[];

    ScopedMalloc( size_t size=0 ) : __parent()
    {
        Reset(size);
    }

    virtual ~ScopedMalloc() throw()
    {
        Reset();
    }

    virtual void Reset( size_t newsize=0 ) override
    {
        xFree(m_buffer);
        m_buffer    = nullptr;
        m_size      = newsize;
        if (!m_size) return;

        m_buffer = (T*)xMalloc( m_size * sizeof(T) );
        bug_on( !m_buffer, "ScopedMalloc failed, requested size=%s bytes", cDecStr(m_size * sizeof(T)) );
    }

    virtual void Resize( size_t newsize ) override
    {
        m_buffer    = (T*)xRealloc(m_buffer, newsize * sizeof(T));

        bug_on( !m_buffer, "Reallocation request failed, old size=%s bytes, requested size=%s bytes",
            cDecStr(m_size * sizeof(T)),
            cDecStr(newsize * sizeof(T))
        );

        m_size      = newsize;
    }
};

// --------------------------------------------------------------------------------------
//  ScopedAlignedMalloc
// --------------------------------------------------------------------------------------
// A simple container class for an aligned allocation.  By default, no bounds checking is
// performed, and there is no option for enabling bounds checking.  If bounds checking and
// other features are needed, use the more robust SafeArray<> instead.
//
// See docs for BaseScopedAlloc for details and rationale.
//
template< typename T, uint align >
class ScopedAlignedMalloc : public BaseScopedMalloc<T>
{
    typedef BaseScopedMalloc<T> __parent;

protected:
    using __parent::m_buffer;
    using __parent::m_size;

public:
    using __parent::operator[];

    ScopedAlignedMalloc( size_t size=0 ) : __parent()
    {
        Reset(size);
    }
    
    virtual ~ScopedAlignedMalloc() throw()
    {
        Reset();
    }

    virtual void Reset( size_t newsize=0 ) override
    {
        xFree_Aligned(m_buffer);
        m_size = newsize;
        if (!m_size) return;

        m_buffer = (T*)xMalloc_Aligned( m_size * sizeof(T), align );
        bug_on( !m_buffer, "aligned malloc failed, requested size=%s bytes", cDecStr(m_size * sizeof(T)) );
    }

    virtual void Resize( size_t newsize ) override
    {
        m_buffer    = (T*)xRealloc_Aligned(m_buffer, newsize * sizeof(T), align);

        bug_on( !m_buffer, "Reallocation request failed, old size=%s bytes, requested size=%s bytes",
            cDecStr(m_size * sizeof(T)),
            cDecStr(newsize * sizeof(T))
        );

        m_size      = newsize;
    }
};
