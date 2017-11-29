
#include "PCH-framework.h"
#include "x-chrono.h"

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

#if TARGET_ORBIS
#	define _stat64 stat
#elif !defined(_MSC_VER)
#	define _stat64 stat64
#endif


#ifndef USE_CUSTOM_HEAP_MSPACE
#	if TARGET_ORBIS
#		define USE_CUSTOM_HEAP_MSPACE		1
#else
#		define USE_CUSTOM_HEAP_MSPACE		0
#	endif
#endif


#if USE_CUSTOM_HEAP_MSPACE

#include <mspace.h>

SceLibcMspace	s_mspace	= nullptr;
off_t			s_physMem	= 0;

void* CreateHeapArea( size_t heapSize, const xString& heapName )
{
	// Use perror() here because our internal assertion handlers haven't been initialized
	// by the time this is invoked.

	int errcode = sceKernelAllocateDirectMemory( 0, SCE_KERNEL_MAIN_DMEM_SIZE,
		heapSize, 0, SCE_KERNEL_WB_ONION, &s_physMem
	);
	if (errcode < 0) {
		perror( "sceKernelAllocateDirectMemory(PS2emu Heap)" );
		__stop();
	}

	void* localMem = nullptr;
	errcode = sceKernelMapDirectMemory( &localMem, heapSize,
		SCE_KERNEL_PROT_CPU_WRITE | SCE_KERNEL_PROT_CPU_READ, SCE_KERNEL_WB_ONION,
		s_physMem, _16kb //, "PS2emu Heap"
	);

	if (errcode < 0) {
		printf_s( "errcode returned: %d\n", errcode );
		printf_s( "errno: %d\n", errno );
		perror( "sceKernelMapNamedDirectMemory(PS2emu Heap)" );
		__stop();
	}

	SceLibcMspace mspace = sceLibcMspaceCreate(heapName, localMem, heapSize, 0);
	if (!mspace) {
		perror( xFmtStr("sceLibcMspaceCreate(%s)", heapName.c_str()) );
		__stop();
	}

	return mspace;
}

void InitCustomHeap( size_t heapSize )
{
	s_mspace = CreateHeapArea(heapSize, "PS2emu Heap");
}

static bool _populateMallocStats(SceLibcMspace mspace, SceLibcMallocManagedSize& msize)
{
	SCE_LIBC_INIT_MALLOC_MANAGED_SIZE(msize);

	if (mspace) {
		int result = sceLibcMspaceMallocStats( s_mspace, &msize );
		if (result && (errno == EINTR)) {
			warn_host("sceLibcMspaceMallocStats returned EINTR.");
			return false;
		}
		abort_perror_on( result, "sceLibcMspaceMallocStats" );
	}
	else {
		auto result = malloc_stats(&msize);
		abort_perror_on( result, "malloc_stats" );

		if (result && (errno == EINTR)) {
			warn_host("malloc_stats returned EINTR.");
			return false;
		}
		abort_perror_on( result, "malloc_stats" );
	}
	return true;
}

void xMalloc_Report()
{
	if (s_mspace) {
		SceLibcMallocManagedSize msize;
		if (_populateMallocStats(s_mspace, msize)) {
			log_bench ( "    xMalloc heap used: %5.1fmb  (of %5.1fmb)", double(msize.currentInuseSize) / _1mb, double(msize.maxSystemSize) / _1mb );
		}
	}

	if (1) {
		SceLibcMallocManagedSize msize;
		if (_populateMallocStats(nullptr, msize)) {
			log_bench ( "    malloc  heap used: %5.1fmb  (of %5.1fmb)", double(msize.currentInuseSize) / _1mb, double(msize.maxSystemSize) / _1mb );
		}
	}

}

// only spams the console if the heap actually changed.
// Typical case in the PS2emu is that there is little or no heap activity.

template<int staticBucket>
static void _reportDelta(SceLibcMspace mspace)
{
	// Hi there, I'm not entirely thread safe.  But it's probably OK enough...!

	static double s_prevSize = 0.0;

	SceLibcMallocManagedSize msize;
	if (!_populateMallocStats(mspace, msize)) {
		// EINTR or similar failure.  Ignore this instance and try again later.
		return;
	}

	double newsize	= double(msize.currentInuseSize) / _1mb;
	double diff		= newsize - s_prevSize;

	if (std::fabs(diff) > 0.001) {
		log_bench ( "    %-7s heap changed: %+5.2fkb, new total = %5.1fmb",
			mspace ? "xMalloc" : "malloc",
			diff * (_1mb / _1kb), newsize
		);
		s_prevSize = newsize;
	}
}

void xMalloc_ReportDelta()
{
	if (s_mspace) { _reportDelta<0>(s_mspace); }
	if (1)        { _reportDelta<1>(nullptr);  }
}

void xMalloc_Check()
{
	SceLibcMallocManagedSize msize;
	SCE_LIBC_INIT_MALLOC_MANAGED_SIZE(msize);
	if (s_mspace) {
		int result = sceLibcMspaceMallocStats( s_mspace, &msize );
		abort_perror_on( result, "sceLibcMspaceMallocStats" );
	}
	else {
		auto result = malloc_stats(&msize);
		abort_perror_on( result, "malloc_stats" );
	}
}

__exi void* xMalloc( size_t sz )
{
	if (s_mspace) {
		void* result = sceLibcMspaceMalloc(s_mspace, sz);
		abort_perror_on( !result, "sceLibcMspaceMalloc(%d)", sz );
		return result;
	}
	else {
		void* result = malloc(sz);
		abort_perror_on( !result, "malloc(%d)", sz );
		return result;
	}
}

__exi void* xCalloc( size_t numItems, size_t sz )
{
	if (s_mspace) {
		void* result = sceLibcMspaceCalloc(s_mspace, 1, numItems * sz);
		abort_perror_on( !result, "sceLibcMspaceCalloc(numItems=%d, sz=%d)", numItems, sz );
		return result;
	}
	else {
		void* result = calloc(1, numItems * sz);
		abort_perror_on( !result, "calloc(numItems=%d, sz=%d)", numItems, sz );
		return result;
	}
}


__exi void* xRealloc( void* srcptr, size_t sz )
{
	if (s_mspace) {
		void* result = sceLibcMspaceRealloc(s_mspace, srcptr, sz);
		abort_perror_on( !result, "sceLibcMspaceRealloc(srcPtr=%s, sz=%d)", cPtrStr(srcptr), sz );
		return result;
	}
	else {
		void* result = realloc(srcptr, sz);
		abort_perror_on( !result, "realloc(srcPtr=%s, sz=%d)", cPtrStr(srcptr), sz );
		return result;
	}
}


__exi void xFree( void *ptr )
{
	// For convenience we allow calls xFree prior to MSPACE initialization if the ptr==null.
	// Some scoped C++ containers (ScopedMalloc, etc) attempt to free nullptr as part of
	// initialization -- which is ok.

	if (!ptr) return;
	if (s_mspace) {
		sceLibcMspaceFree(s_mspace, ptr);
	}
	else {
		free(ptr);
	}
}


__exi void* xMalloc_Aligned( size_t sz, u32 align )
{
	if (align <= 32) {
		return xMalloc( sz );
	}

	if (s_mspace) {
		void* result = sceLibcMspaceMemalign(s_mspace, align, sz);
		abort_perror_on( !result, "sceLibcMspaceMemalign(sz=%d, align=%d)", sz, align );
		return result;
	}
	else {
		void* result = memalign(align, sz);
		abort_perror_on( !result, "memalign(sz=%d, align=%d)", sz, align );
		return result;
	}
}


__exi void* xRealloc_Aligned( void* srcptr, size_t sz, u32 align )
{
	if (s_mspace) {
		void* result = sceLibcMspaceReallocalign(s_mspace, &srcptr, align, sz);
		abort_perror_on( !result, "sceLibcMspaceReallocalign(srcPtr=%s, sz=%d, align=%d)", cPtrStr(srcptr), sz, align );
		return result;
	}
	else {
		void* result = reallocalign(&srcptr, align, sz);
		abort_perror_on( !result, "reallocalign(srcPtr=%s, sz=%d, align=%d)", cPtrStr(srcptr), sz, align );
		return result;
	}
}

__exi void xFree_Aligned( void* ptr )
{
	xFree(ptr);
}
#else

void InitCustomHeap( size_t heapSize ) { }

#if TARGET_ORBIS
// --------------------------------------------------------------------------------------
__exi void xFree( void *ptr )
{
	free(ptr);
}

__exi void* xMalloc( size_t sz )
{
	return malloc(sz);
}

__exi void* xCalloc( size_t numItems, size_t sz )
{
	return calloc(1, numItems * sz);
}

__exi void* xRealloc( void* srcptr, size_t sz )
{
	return realloc(srcptr, sz);
}

// --------------------------------------------------------------------------------------
__exi void* xMalloc_Aligned( size_t sz, u32 align )
{
	// ORBIS 0.820: posix_memalign() does not work, or more specifically only works
	// when align is 32 (which is the implicit align of all malloc heap anyway). --jstine
	// memalign() however does work as expected.  Interesting!

	void* result = memalign(align,sz);
	return result;
}

#else

// --------------------------------------------------------------------------------------
// Windows and Linux don't match PS4's implicit alignment restriction (which is 32 bytes).
// For ease of cross-platform development, force all standard malloc requests to be 32-byte
// aligned.

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
#endif		// #if TARGET_ORBIS / #else


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
	// Not especially relevant on non-PS4 platforms
}

void xMalloc_ReportDelta()
{
}

#endif

// ======================================================================================
// ======================================================================================

// --------------------------------------------------------------------------------------
bool xFileExists( const char* fullpath )
{
	if (!fullpath || !fullpath[0]) return false;

	xString fixed = xFixFilenameForPS4(fullpath);

#if TARGET_ORBIS
	// sceKernelCheckReachability -- explained:
	//   Someone decided it was a good idea to use a highly optimized hash for the file system
	//   on PS4, which makes the following assumptions:
	//    1. no collisions will ever occur.
	//    2. files always exist.
	//
	//   Therefore, when using open() or stat() to check a file that does not exist, another file
	//   will be opened in its place and thus open/stat will always return success.  Furthermore,
	//   it is possible (but rare) that the wrong file is opened due to hash collision.
	//   So SCE introduced sceKernelCheckReachability() which does not make the assumption that
	//   the file exists, and also repairs collided hash entries by some method.
	//
	//   However, it's only "supposed" to be used for app0 and addcont portions of the PS4 file
	//   system.  Its behavior when used to access network shares via the host PC is maybe not
	//   so reliable.  Let's hope future PS4 sys doesn't decide to extend this hashed FS to other
	//   folders of the system!
	//  --jstine, 07 july 2014
	//
	if (fixed.StartsWith("/app0/") || fixed.StartsWith("/addcont")) {
		int result = sceKernelCheckReachability(fixed);
		return result == SCE_OK;
	}

#endif

	struct _stat64 sinfo;
	return( _stat64 (fixed, &sinfo ) != -1 );
}

// --------------------------------------------------------------------------------------
void xFileSetSize( int fd, size_t filesize )
{
#if TARGET_ORBIS
	int result = sceKernelFtruncate(fd, filesize);
	if (result < 0)
	{
		log_and_abort("Failed to set file size to %s bytes, error 0x%08x",
			cDecStr(filesize), result
		);
	}
#else
	#if defined(_MSC_VER)
		int error	= _chsize_s(fd, filesize);
		int result	= error ? -1 : 0;
	#else
		int result = ftruncate(fd, filesize);
		int error  = errno;
	#endif
		if (result==-1) {
			log_and_abort("Failed to set file size to %s bytes, error #%d: %s",
				cDecStr(filesize), error, xPosixErrorStr(error).c_str()
			);
		}
	#endif
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

	const xString src  (xFixFilenameForPS4(src_));
	const xString dest (xFixFilenameForPS4(dest_));

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

#if TARGET_ORBIS
	int result = sceKernelMkdir( dir, SCE_KERNEL_S_IRWU );
	if (result == SCE_OK)					return true;
	if (result == SCE_KERNEL_ERROR_EEXIST) 	return true;
	return false;
#else
	int result = _mkdir( dir );

	if (!result)			return true;
	if (errno == EEXIST)	return true;
	return false;
#endif
}

// --------------------------------------------------------------------------------------
// Creates entire hierarchy of requested directory trees, if the path is:
//   A. relative to the CWD (subdir)
//   B. located under /tmp/ or /download*  (PS4)
//
bool xCreateDirectory( const xString& origDir )
{
	if (origDir.IsEmpty()) return true;

	xString dir = xFixFilenameForPS4(origDir);
	bool allowNestedCreation = !xPathIsAbsolute(dir);

#if TARGET_ORBIS
	if (	origDir	.StartsWith("/tmp/"			)
		||	origDir	.StartsWith("/download0/"	)
		||  dir		.StartsWith("/hostapp/"		)
		||  dir		.StartsWith("/app0/"		)
	) {
		allowNestedCreation = true;
	}

//	if (dir.StartsWith("/host/"		))	allowNestedCreation = false;
#endif

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
	auto result = fopen_s( &fp, xFixFilenameForPS4(fullpath), mode );
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

host_tick_t		HostClockTick::s_ticks_per_second;
double			HostClockTick::s_ticks_per_second_f;

void HostClockTick::Init()
{
	s_ticks_per_second		= Host_GetProcessTickRate();
	s_ticks_per_second_f	= s_ticks_per_second;
}
