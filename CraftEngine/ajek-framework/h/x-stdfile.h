#pragma once

#include "x-types.h"
#include "x-stdalloc.h"

#include <cstdio>

// ======================================================================================
//  xBaseStream (abstract class)
// ======================================================================================
class xBaseStream
{
    NONCOPYABLE_OBJECT( xBaseStream );

protected:
    union {
        // We cheat her a bit -- m_fd strictly speaking should be -1 for an empty descriptor.
        // The API manually sets -1 to zero (0) within OpenFile() to allow !nullptr checks
        // against m_fp to satisfy either underlying file system.  This is ok because posix
        // fd==0 is reserved as stdin anyway, and will never be returned from open(). --jstine

        FILE*       m_fp;               // non-NULL implies data is sourced from file
        int         m_fd;
    };

    size_t              m_dataPos       = 0;
    size_t              m_dataLength    = 0;
    bool                m_isBuffered    = false;            // if TRUE, use m_fp else use m_fd
    xCustomAllocator*   m_allocator     = &g_DefaultCustomAllocator;

protected:
    xBaseStream() {
        m_fp            = nullptr;
    }

public:
    virtual             ~xBaseStream    () throw()=0;
    virtual void        Close           ();
    virtual const void* GetBufferPtr    (size_t size) const=0;
    virtual void*       GetBufferPtr    (size_t size)=0;
            x_off_t     Seek            (x_off_t pos, int whence);
            x_off_t     Tell            ()              const;

            void        SetCustomAlloc  (xCustomAllocator& allocator)   { m_allocator = &allocator; }
            bool        IsMemory        ()              const   { return  !m_fp;  }
            bool        IsFile          ()              const   { return !!m_fp; }
            bool        IsOK            ()              const   { return !!m_fp || !!m_dataLength; }
            FILE*       GetFILE         ()                      { return m_isBuffered ? m_fp : nullptr; }
};

inline xBaseStream::~xBaseStream() throw() {}


// ======================================================================================
//  xStreamReader (class)
// ======================================================================================
class xStreamReader : public xBaseStream
{
    typedef xBaseStream __parent;

protected:
    const u8*   m_data;

public:
                        xStreamReader   ()          { }
    virtual             ~xStreamReader  () throw() { Close(); }
    virtual void        Close           ()                          override;
    virtual const void* GetBufferPtr    (size_t size) const         override;
    virtual void*       GetBufferPtr    (size_t size)               override;
            bool        OpenFile        ( const xString& filename );
            bool        OpenFileBuffered( const xString& filename );
            bool        OpenMem         ( const void* mem, size_t buffer_length );
            bool        Read            ( void* dest, ssize_t bytes );

    // Valid for both file and data streams in xStreamReader.
            size_t      GetLength       ()              const   { return m_dataLength; }

    template< typename T >
    bool Read( T& dest )
    {
        serialization_assert(T);
        return Read( &dest, sizeof(T) );
    }
};

struct xStatInfo
{
    u32         st_mode;
    s64         st_size;

    time_t      time_accessed;
    time_t      time_modified;
    time_t      time_created;

    bool IsFile     () const;
    bool IsDir      () const;
    bool Exists     () const;

};

extern xStatInfo    xFileStat               (const xString& path);
extern bool         xFileRename             (const xString& src, const xString& dst);
extern void         xFileUnlink             (const xString& src);
extern bool         xCreateDirectory        (const xString& dir);
extern FILE*        xFopen                  (const xString& fullpath, const char* mode);

extern void         xFileSetSize            (int fd, size_t filesize);
extern bool         xFgets                  (xString& dest, FILE* stream);


// Writes directly to debug console where supported (visual studio), or stdout otherwise.
extern void     xOutputString       (const char* str, FILE* std_fp);

