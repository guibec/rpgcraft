
#include "PCH-framework.h"

#include "x-types.h"
#include "x-chrono.h"
#include "x-stdfile.h"

#if USE_GLIBC_MACRO_FIXUP
#   undef __unused
#   undef __xi
#endif

#include <sys/stat.h>   // for xFileExists and stuff

#if TARGET_MSW
#   include <malloc.h>      // Just for _aligned_malloc(windows)
#   include <direct.h>
#   include <io.h>          // for _chsize_s
#else
#   include <sys/types.h>
#   include <unistd.h>      // needed for ftruncate on newer glibc 2.22+ (gcc 4.7+)
#   define _mkdir(folder)   mkdir(folder, S_IRWXU | S_IRWXG) // | S_IROTH | S_IXOTH)
#endif

#if USE_GLIBC_MACRO_FIXUP
#   define __unused     __UNUSED
#   define __xi         __ALWAYS_INLINE_FUNCTION
#endif

#include "x-MemCopy.inl"

#if defined(__clang__)
#   define _stat64 stat
#elif !defined(_MSC_VER)
#   define _stat64 stat64
#endif


// --------------------------------------------------------------------------------------
__exi void xFree( void *ptr )
{
    xFree_Aligned(ptr);
}

__exi void* xMalloc( size_t sz )
{
    return xMalloc_Aligned(sz, 32);
}

__exi void* xCalloc( size_t numItems, size_t sz )
{
    void* result = xMalloc_Aligned(sz * numItems, 32);
    memset(result, 0, sz * numItems);
    return result;
}

__exi void* xRealloc( void* srcptr, size_t sz )
{
    return xRealloc_Aligned(srcptr, sz, 32);
}

// --------------------------------------------------------------------------------------
__exi void* xMalloc_Aligned( size_t sz, u32 align )
{
#if TARGET_MSW
    return _aligned_malloc(sz, align);
#else
    void* result;
    __unused int code = posix_memalign(&result, align, sz);
    bug_on_qa (code, "posix_memalign failed with an error: %u", code );
    return result;
#endif
}

// --------------------------------------------------------------------------------------
__exi void* xRealloc_Aligned( void* srcptr, size_t sz, u32 align )
{
#ifndef _MSC_VER
    // posix says realloc should be able to handle aligned pointers correctly.
    return realloc(srcptr, sz);
#else
    return _aligned_realloc(srcptr, sz, align);
#endif
}

// --------------------------------------------------------------------------------------
__exi void xFree_Aligned( void* ptr )
{
#ifndef _MSC_VER
    if (!ptr) return;
    free(ptr);
#else
    _aligned_free(ptr);
#endif
}

void xMalloc_Report()
{
}

void xMalloc_ReportDelta()
{
}

// ======================================================================================
// ======================================================================================


bool xStatInfo::IsFile     () const { return st_mode & S_IFREG; }
bool xStatInfo::IsDir      () const { return st_mode & S_IFDIR; }
bool xStatInfo::Exists     () const { return st_mode & (S_IFDIR | S_IFREG); }

// --------------------------------------------------------------------------------------
xStatInfo xFileStat(const xUniPath& upath)
{
    if (upath.IsEmpty()) return {};
    struct _stat64 sinfo;

    if (_stat64 (upath.GetLibcStr(), &sinfo ) == -1) return {};

    return {
        sinfo.st_mode,
        sinfo.st_size,
        sinfo.st_atime,
        sinfo.st_mtime,
        sinfo.st_ctime
    };
}

// --------------------------------------------------------------------------------------
void xFileSetSize( int fd, size_t filesize )
{
#if defined(_MSC_VER)
    int error	= _chsize_s(fd, filesize);
    int result	= error ? -1 : 0;
#else
    int result = ftruncate(fd, filesize);
    int error  = errno;
#endif
    if (result==-1) {
        x_abort("Failed to set file size to %s bytes, error #%d: %s",
            cDecStr(filesize), error, cPosixErrorStr(error)
        );
    }
}

// --------------------------------------------------------------------------------------
bool xFileRename( const xUniPath& src_, const xUniPath& dest_ )
{
#if !TARGET_MSW
    // On Windows rename errors if a file exists.  Linux/POSIX however just clobber
    // the existing file instead.  I prefer windows since what I want is an atomic
    // rename-if-no-existing-file function.  This work-around isn't robust against running
    // multiple instances in the same runenv.  An improved strategy would attempt to create
    // the destination file, instead of checking if it already exists.

    if (xFileExists(dest_)) return false;
#endif

    auto& src  = src_ .GetLibcStr();
    auto& dest = dest_.GetLibcStr();

    int result = rename( src, dest );
    if (!result) return true;

    switch(errno)
    {
        case ENOENT:    return true;    // responsibility of caller to care if source exists or not...

        case EEXIST:
        case ENOTEMPTY:
        case EACCES:    return false;   // dest file already exists.

        case EBUSY:
            warn( "Could not complete rename request: source or dest is .. 'busy'." );
        return false;

        case ELOOP:
        case EMLINK:
            warn( "Could not complete rename request: symbolic linking error occured." );
        return false;

        case EINVAL:
            x_abort(
                "xFileRename failed due to invalid parameters. One or both filenames may contain invalid characters.\n"
                "  src-host : %s\n"
                "  src-unix : %s\n"
                "  dst-host : %s\n"
                "  dst-unix : %s\n",
                src_ .GetLibcStr().c_str(),
                src_ .asUnixStr().c_str(),
                dest_.GetLibcStr().c_str(),
                dest_.asUnixStr().c_str()
            );
        break;
    }

    // Some other unconsidered posix error condition.
    return false;
}

// --------------------------------------------------------------------------------------
// Custom-rolled fgets!  This handles line endings very differently:
//  * Mac-style CR linefeeds are recognized, and stripped.
//  * DOS-style CR+LF and Unix-style LF are stripped.
//
// For best performance, dest should be a reused string so to avoid redundant heap
// allocations.
//
// Returns FALSE if the stream is EOF.
//
bool xFgets(xString& dest, FILE* stream)
{
    if (feof(stream)) return false;

    // Dev note: do not add support for escaping EOLs or quotes.
    // Parsing those sort of things should be the job of the caller because there's
    // far to many nuanced behavioral choices about how to handle those.

    bool isCR = false;
    while (1)
    {
        int ch = fgetc(stream);
        if (ch == EOF)  break;
        if (ch == 0)    break;      // may indicate EOF on some platforms, or maybe no

        // CR and CR/LF handling: if the next char is a /n then break the line there.
        // But if it's not a /n then break *and* rewind.  That char will want to belong
        // to the next string.

        if  (ch == '\n') break;
        elif(ch == '\r')
        {
            isCR = true;
            continue;
        }
        elif(isCR)
        {
            fseek( stream, -1, SEEK_CUR );
            break;
        }

        dest += ch;
    }

    return true;
}

bool _createDirectory( const char* dir )
{
    struct _stat64i32 info;
    if (_stat(dir, &info) == -1) {
        auto code = errno;
        if (code == ENOENT) {
            int result = _mkdir( dir );
            if (!result) return true;
            bug_on (errno == EEXIST);
            return false;
        }

        bug("_stat() failed - %s", cPosixErrorStr(code));
    }

    return ((info.st_mode & _S_IFDIR) == _S_IFDIR);
}

// Creates entire hierarchy of requested directory trees.
bool xCreateDirectory( const xUniPath& orig_unix_dir )
{
    if (orig_unix_dir.IsEmpty()) return true;

    const auto& dir  = orig_unix_dir.m_unixpath;
    bool isAbsolute = xPathIsAbsolute(dir);

    static const char* dirseps  = "/";
    static const char* sepChar  = "/";
    static const char* uriCheck = "/";

    int tokenSkip = 0;
    int pos = 0;

    // check for network URI and skip root if it exists.
    if (dir.StartsWith("//")) {
        tokenSkip = 2;
        pos = 2;
    }
    if(TARGET_MSW && dir[0] == '/' && isalnum((u8)dir[1])) {
        tokenSkip = 1;
        pos = 2;
    }

    bool firstToken = 1;
    bool isTangible = 0;        // set to 1 once path becomes something other than ./../.. (so on)

    for(; pos < dir.GetLength() && dir[pos]; ++pos) {
        if (dir[pos] == '/') {
            if (!pos) {
                continue;
            }

            if (tokenSkip) {
                --tokenSkip;
                continue;
            }

            xString dirdup = dir.GetSubstring(0, pos);
            isTangible = isTangible || !(
                (dirdup.EndsWith("/.")   ) ||
                (dirdup.EndsWith("/..")  ) ||
                (dirdup == "."           ) ||
                (dirdup == ".."          )
            );

            if (isTangible) {
                if (!_createDirectory(orig_unix_dir.GetLibcStr(dirdup))) {
                    return false;
                }
            }
        }
    }
    return _createDirectory(orig_unix_dir.GetLibcStr());
}

FILE* xFopen( const xString& fullpath, const char* mode )
{
    FILE* fp;
    auto result = fopen_s( &fp, xUniPathInit(fullpath).GetLibcStr(), mode );
    if (result) { fp = nullptr; }
    return fp;
}

xStatInfo xFileStat(const xString& path)
{
    return xFileStat(xUniPathInit(path));
}

bool xFileRename(const xString& src_, const xString& dest_)
{
    return xFileRename(xUniPathInit(src_), xUniPathInit(dest_));
}

bool xCreateDirectory( const xString& dir ) {
    return xCreateDirectory(xUniPathInit(dir));
}


//**************************************************************************

xString DecodeBitField(u32 bits, const char* pLegend[], u32 numEntries)
{
    // Decode a bitfield according to the array of strings passed in.
    // Legend should be ordered from least-significant bit to most,
    // with non-NULL entry for every potentially non-zero bit
    // Returns pBuffer for syntactic convenience
    if (!pLegend || !numEntries) return xString();

    const char** ppKey   = pLegend;
    bool         doSpace = false;
    xString      str;

    while(bits != 0 && numEntries > 0) {
        if (bits & 1) {
            if (*ppKey) {
                if (doSpace) str += ' ';
                doSpace = true;
                str    += *ppKey;
            }
        }
        ppKey++;
        bits >>= 1;
        numEntries--;
    }

    return str;
}

// --------------------------------------------------------------------------------------
__exi void xMemCopyQwc_WrappedDest( u128* destBase, const u128* src, uint& destStartQwc, uint lenQwc, uint destSizeQwc )
{
    uint endpos  = destStartQwc + lenQwc;
    if ( endpos <= destSizeQwc )
    {
        xMemCopyQwc( &destBase[destStartQwc], src, lenQwc );
        destStartQwc = (destStartQwc + lenQwc) % destSizeQwc;
    }
    else
    {
        uint firstcopylen = destSizeQwc - destStartQwc;
        xMemCopyQwc( &destBase[destStartQwc], src, firstcopylen );
        destStartQwc = endpos - destSizeQwc;
        bug_on  (destStartQwc > destSizeQwc);
        xMemCopyQwc(destBase, src+firstcopylen, destStartQwc );
    }
}

__exi void xMemCopyQwc_WrappedSrc( u128* dest, const u128* srcBase, uint& srcStartQwc, uint lenQwc, uint srcSizeQwc )
{
    uint endpos  = srcStartQwc + lenQwc;
    if ( endpos <= srcSizeQwc )
    {
        xMemCopyQwc(dest, &srcBase[srcStartQwc], lenQwc );
        srcStartQwc = (srcStartQwc + lenQwc) % srcSizeQwc;
    }
    else {
        uint firstcopylen = srcSizeQwc - srcStartQwc;
        xMemCopyQwc(dest, &srcBase[srcStartQwc], firstcopylen );
        srcStartQwc = endpos - srcSizeQwc;
        bug_on  (srcStartQwc > srcSizeQwc);
        xMemCopyQwc(dest+firstcopylen, srcBase, srcStartQwc );
    }
}

void xStrCopy(char* dest, size_t destLen, const char* src)
{
#if TARGET_LINUX
    strncpy(dest, src, destLen);
#else
    strcpy_s( dest, destLen, src);
#endif
}


void xStrnCopy(char* dest, size_t destLen, const char* src, size_t srcLen)
{
#if TARGET_LINUX
    strncpy(dest, src, std::min(destLen, srcLen));
#else
    strncpy_s( dest, destLen, src, srcLen );
#endif
}


// x-simd.h tihings  (not in a mood to put these in their own module, yet...)

uint2::operator int2 () const {
#if UNSIGNED_OVERFLOW_CHECK
    bug_on(int(x) < 0 || int(y) < 0, "Unsigned integer overflow detected.");
#endif
    return { int(x), int(y) };
}

// x-chrono.h things  (not in a mood to put these in their own module, yet...)

host_tick_t     HostClockTick::s_ticks_per_second;
double          HostClockTick::s_ticks_per_second_f;

void HostClockTick::Init()
{
    s_ticks_per_second      = Host_GetProcessTickRate();
    s_ticks_per_second_f    = s_ticks_per_second;
}

