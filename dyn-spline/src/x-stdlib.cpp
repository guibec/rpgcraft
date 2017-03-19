
#include "common-standard.h"

#if USE_GLIBC_MACRO_FIXUP
#	undef __unused
#	undef __xi
#endif

#include <sys/stat.h>	// for xFileExists and stuff

#if TARGET_MSW
#	include <malloc.h>		// Just for _aligned_malloc(windows)
#	include <direct.h>
#	include <io.h>			// for _chsize_s
#else
#	include <sys/types.h>
#	include <unistd.h>		// needed for ftruncate on newer glibc 2.22+ (gcc 4.7+)
#	define _mkdir(folder)	mkdir(folder, S_IRWXU | S_IRWXG) // | S_IROTH | S_IXOTH)
#endif

#if USE_GLIBC_MACRO_FIXUP
#	define __unused		__UNUSED
#	define __xi			__ALWAYS_INLINE_FUNCTION
#endif

DECLARE_MODULE_NAME( "x-stdlib" );

#include "x-MemCopy.inl"

#if defined(__clang__)
#	define _stat64 stat
#elif !defined(_MSC_VER)
#	define _stat64 stat64
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

// --------------------------------------------------------------------------------------
bool xFileExists( const char* fullpath )
{
	if (!fullpath || !fullpath[0]) return false;

	xString fixed = xFixFilenameForPlatform(fullpath);
	struct _stat64 sinfo;
	return( _stat64 (fixed, &sinfo ) != -1 );
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
		log_and_abort("Failed to set file size to %s bytes, error #%d: %s",
			cDecStr(filesize), error, cPosixErrorStr(error)
		);
	}
}

// --------------------------------------------------------------------------------------
bool xFileRename( const xString& src_, const xString& dest_ )
{
#if !TARGET_MSW
	// On Windows rename errors if a file exists.  Linux/POSIX however just clobber
	// the existing file instead.  I prefer windows since what I want is an atomic
	// rename-if-no-existing-file function.  This work-around isn't robust against running
	// multiple instances in the same runenv.

	if (xFileExists(dest_)) return false;
#endif

	const xString src  (xFixFilenameForPlatform(src_));
	const xString dest (xFixFilenameForPlatform(dest_));

	int result = rename( src, dest );
	if (!result) return true;

	switch(errno)
	{
		case ENOENT:	return true;	// responsibility of caller to care if it exists or not...

		case EEXIST:
		case ENOTEMPTY:
		case EACCES:	return false;	// dest file already exists.

		case EBUSY:
			warn( "Could not complete rename request: source or dest is .. 'busy'." );
		return false;

		case ELOOP:
		case EMLINK:
			warn( "Could not complete rename request: symbolic linking error occured." );
		return false;

		case EINVAL:
			log_and_abort(
				"Invalid parameters, one or both filenames contain invalid characters...\n"
				"srcfile : %s\n"
				"destfile: %s",
				src.c_str(),
				dest.c_str()
			);
		break;
	}

	// Some other unconsidered posix error condition.
	return false;
}

// --------------------------------------------------------------------------------------
// Looks for the next filename available in the form of:
//   {destPathAndFileBase}xxxx{ext} -->  /path/to/padlog0003.txt
// ... and renames srcFullPathname into that slot.
//
// Returns TRUE (success) if srcFullPathName does not exist, or is successfully renamed.
// Returns FALSE only if no suitable slot could be found (which should never happen).
//
bool xFileSystematicRename( const xString& srcFullPathname, const xString& destPathAndFileBase, const xString& ext, int checkLimit )
{
	if (!xFileExists(srcFullPathname)) return true;

	xString destTry;
	int attemptCounter = 0;

	while(1)
	{
		// [TODO] if we assume a limit of 9999 (which is reasonable) then this could
		//        be optimized to just write the xxxx number in-place without re-forming
		//        the whole string each time.
		destTry.Format( "%s%04u%s", destPathAndFileBase.c_str(), attemptCounter, ext.c_str());
		if (xFileRename(srcFullPathname, destTry))
			break;

		if (++attemptCounter > checkLimit)
		{
			log_host( "Could not rename '%s' -- all destination slots are full!", srcFullPathname.c_str() );
			return false;
		}
	}

	log_host( "Renamed '%s' -> '%s'", srcFullPathname.c_str(), destTry.c_str() );
	return true;
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
// [TODO] Add support for multi-line quoted strings and backslashed (escaped) EOL!
//
bool xFgets(xString& dest, FILE* stream)
{
	if (feof(stream)) return false;

	bool isCR = false;
	while (1)
	{
		int ch = fgetc(stream);
		if (ch == EOF)	break;
		if (ch == 0)	break;		// may indicate EOF on some platforms, or maybe no

		// CR and CR/LF handling: if the next char is a /n then break the line there.
		// But if it's not a /n then break *and* rewind.  That char will want to belong
		// to the next string.

		if	(ch == '\n') break;
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

bool _createDirectory( const xString& dir )
{
	// EEXIST isn't always a good thing -- if its a DIR it is good, but if the existing
	// entity is a file then its bad.  Too much work to differentiate tho (need to use
	// stat). --jstine

	int result = _mkdir( dir );

	if (!result)			return true;
	if (errno == EEXIST)	return true;
	return false;
}

// --------------------------------------------------------------------------------------
// Creates entire hierarchy of requested directory trees, if the path is:
//   A. relative to the CWD (subdir)
//   B. located under /tmp/ or /download*  (PS4)
//
bool xCreateDirectory( const xString& origDir )
{
	if (origDir.IsEmpty()) return true;

	xString dir = xFixFilenameForPlatform(origDir);
	bool allowNestedCreation = !xPathIsAbsolute(dir);

	if (!allowNestedCreation) {
		return _createDirectory(dir);
	}

	static const char* dirseps = TARGET_LINUX ? "/" : "\\/";

	xStringTokenizer split(dirseps, dir);
	xString totalConcat;

	const char* sepChar = TARGET_MSW ? "\\" : "/";
	while(split.HasMoreTokens()) {
		xString thisToken = split.GetNextToken();
		if (thisToken.IsEmpty())
		{
			if(totalConcat.IsEmpty()) {
				totalConcat = sepChar;
			}
			continue;
		}
		if (thisToken == ".") continue;
		totalConcat = xPath_Combine(totalConcat, thisToken);
		bool result = _createDirectory(totalConcat);
		if (!result) return false;
	}
	return true;
}

FILE* xFopen( const xString& fullpath, const char* mode )
{
	FILE* fp;
	auto result = fopen_s( &fp, xFixFilenameForPlatform(fullpath), mode );
	if (result) { fp = nullptr; }
	return fp;
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


void xStrCopy(char* dest, size_t destLen, const char* src, size_t srcLen)
{
#if TARGET_LINUX
	strncpy( dest, src, destLen );
#else
	strncpy_s( dest, destLen, src, srcLen );
#endif
}


__exi xString xFixFilenameForPlatform(const xString& src)
{
	return src;
}