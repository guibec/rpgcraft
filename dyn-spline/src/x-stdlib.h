
#pragma once

#include "x-types.h"
#include "x-simd.h"
#include "x-stl.h"

#include <cstring>		// needed for memset
#include <type_traits>
#include <cstdio>

#define serialization_assert(T) \
	static_assert(__is_trivially_copyable(T), "Cannot serialize non-POD object.")

#if TARGET_LINUX
#	define printf_s			printf
#	define fprintf_s		fprintf
#	define vfprintf_s		vfprintf

#	define fopen_s(hptr, fpath, mode) \
		((log_abort_on(!(hptr)), (*(hptr) = fopen(fpath, mode))) == nullptr)
#endif


// ======================================================================================
//  FunctHashIdentity  (functor)
// ======================================================================================
// Helper class for use with std:unordered_map<>, when it is desirable to avoid storing the
// hashed data into the container.
//
class FunctHashIdentity {
public:
	__xi size_t operator()(const s32& input) const {
		return input;
	}

	__xi size_t operator()(const u32& input) const {
		return input;
	}

	__xi size_t operator()(const u64& input) const {
		return input;
	}
};


// ======================================================================================
//  xCustomAllocator  (interface)
// ======================================================================================
class xCustomAllocator {
public:
	virtual void*		alloc		(size_t size)				=0;
	virtual void*		realloc		(void* ptr, size_t size)	=0;
	virtual void		free		(void* ptr)					=0;
};

class xDefaultCustomAllocator : public xCustomAllocator {
public:
	void*		alloc		(size_t size)				override;
	void*		realloc		(void* ptr, size_t size)	override;
	void		free		(void* ptr)					override;
};

extern xDefaultCustomAllocator	g_DefaultCustomAllocator;

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

		FILE*		m_fp;				// non-NULL implies data is sourced from file
		int			m_fd;
	};

	size_t				m_dataPos		= 0;
	size_t				m_dataLength	= 0;
	bool				m_isBuffered	= false;			// if TRUE, use m_fp else use m_fd
	xCustomAllocator*	m_allocator		= &g_DefaultCustomAllocator;

protected:
	xBaseStream() {
		m_fp			= nullptr;
	}

public:
	virtual				~xBaseStream	() throw()=0;
	virtual void		Close			();
	virtual	const void*	GetBufferPtr	(size_t size) const=0;
	virtual	void*		GetBufferPtr	(size_t size)=0;
			x_off_t 	Seek			(x_off_t pos, int whence);
			x_off_t 	Tell			()				const;

			void		SetCustomAlloc	(xCustomAllocator& allocator)	{ m_allocator = &allocator; }
			bool		IsMemory		()				const	{ return  !m_fp;  }
			bool		IsFile			()				const	{ return !!m_fp; }
			bool		IsOK			()				const	{ return !!m_fp || !!m_dataLength; }
			FILE*		GetFILE			()						{ return m_isBuffered ? m_fp : nullptr; }
};

inline xBaseStream::~xBaseStream() throw() {}


// ======================================================================================
//  xStreamReader (class)
// ======================================================================================
class xStreamReader : public xBaseStream
{
	typedef xBaseStream __parent;

protected:
	const u8*	m_data;

public:
						xStreamReader	()			{ }
	virtual				~xStreamReader	() throw() { Close(); }
	virtual	void		Close			()							override;
	virtual	const void*	GetBufferPtr	(size_t size) const			override;
	virtual	void*		GetBufferPtr	(size_t size)				override;
			bool		OpenFile		( const xString& filename );
			bool		OpenFileBuffered( const xString& filename );
			bool		OpenMem			( const void* mem, size_t buffer_length );
			bool		Read			( void* dest, ssize_t bytes );

	// Valid for both file and data streams in xStreamReader.
			size_t		GetLength		()				const	{ return m_dataLength; }

	template< typename T >
	bool Read( T& dest )
	{
		serialization_assert(T);
		return Read( &dest, sizeof(T) );
	}
};


// ======================================================================================
//  xStreamWriter (class)
// ======================================================================================
class xStreamWriter : public xBaseStream
{
	typedef xBaseStream __parent;

protected:
	u8*			m_data;
	size_t		m_dataReserved;
	size_t		m_alignMask;
	bool		m_suppress_assertions;

public:
						xStreamWriter	()			{ m_data = nullptr; m_dataReserved = 0; }
	virtual				~xStreamWriter	() throw()	{ Close(); }
	virtual	void		Close			()							override;
	virtual	const void*	GetBufferPtr	(size_t size) const			override;
	virtual	void*		GetBufferPtr	(size_t size)				override;
			bool		CreateFile		(const xString& filename);
			bool		CreateFileBuffered(const xString& filename);
			bool		OpenFile		(const xString& filename);
			bool		OpenFileBuffered(const xString& filename);
			bool		OpenMem			(size_t initial_length, size_t align_by=_64kb);

			void		Flush			();
			bool		Write			( const void* src, ssize_t bytes );
			bool		WriteChars		( const xString& src );
			bool		CopyToStream	( xStreamWriter& dest );

	template< typename T >
	bool Write( const T& src )
	{
		serialization_assert(T);
		return Write( &src, sizeof(T) );
	}

protected:
			void		_growPast		(size_t pos);
};


extern void		xMalloc_Check		();
extern void*	xMalloc				(size_t sz);
extern void*	xCalloc				(size_t numItems, size_t sz);
extern void*	xRealloc			(void* srcptr, size_t sz);
extern void		xFree				(void *ptr);
extern void*	xMalloc_Aligned		(size_t sz, u32 align);
extern void*	xRealloc_Aligned	(void* srcptr, size_t sz, u32 align);
extern void		xFree_Aligned		(void* ptr);
extern void		xMalloc_Report		();
extern void		xMalloc_ReportDelta	();


#define placement_new(T)		new (xMalloc(sizeof(T))) T

template<typename T>
void placement_delete(T* ptr)	{ (ptr)->~T(); xFree(ptr); }

extern void		xStrCopy				(char* dest, size_t destLen, const char* src, size_t srcLen);

extern void		xMemCopy				(void* dest, const void* src, uint len);
extern void		xMemCopy32				(void* dest, const void* src, uint len32);
extern void		xMemCopyQwc				(void* dest, const void* src, uint lenQwc);
extern void		xMemCopyQwc_WrappedDest	(u128* destBase,	const u128* src,		uint& destStartQwc,	uint lenQwc, uint destSizeQwc);
extern void		xMemCopyQwc_WrappedSrc	(u128* dest,		const u128* srcBase,	uint& srcStartQwc,	uint lenQwc, uint srcSizeQwc);

extern void		xMemCopyShortQwc		(void* dest, const void* src, uint lenQwc);
extern void		xMemCopyShortQwc_NT		(void* dest, const void* src, uint lenQwc);

extern void		xFileDelete				(const xString& file);
extern void		xFileSetSize			(int fd, size_t filesize);
extern bool		xFileExists				(const char* fullpath);
extern bool		xFileRename				(const xString& src, const xString& dest);
extern bool		xFileSystematicRename	(const xString& srcFullPathname, const xString& destPathAndFileBase, const xString& ext, int checkCount);

extern bool		xFgets					(xString& dest, FILE* stream);
extern bool		xCreateDirectory		(const xString& dir);
extern FILE*	xFopen					(const xString& fullpath, const char* mode);

template< typename T >
inline void xMemMove(T* dest, size_t destLen, const T* src, size_t srcLen)
{
#if TARGET_LINUX
	memmove( dest, src, srcLen );
#else
	memmove_s( dest, destLen, src, srcLen );
#endif
}

template<size_t destSize>
void xStrCopyT(char (&dest)[destSize], const char* src, size_t srcLen)
{
	xStrCopy(dest, destSize, src, srcLen);
}

// ======================================================================================
#ifdef _MSC_VER
	typedef unsigned long*	_stos_s32;
#else
	typedef u32*			_stos_s32;
#endif

template< u32 t_size >
inline __ai void _internal_memzero( void* dest )
{
	// notes:
	//  # don't use u128's helpers since it assumes alignment and generates buserr.
	//  # Clang/LLVM on PS4 does a good job optimizing short memsets already, but still
	//    doesn't use rep stosd for long copies (invokes a call to memset).
	//  # Clang/LLVM also refuses to store XMMs to anything where the alignment is
	//    indetermininate.

	__m128* d128 = (__m128*)dest;
	__m128 zerox = _mm_setzero_ps();

	switch (t_size)
	{
		case 1:  (u8&)   d128[0] = 0;			break;
		case 2:  (u16&)  d128[0] = 0;			break;
		case 4:  (u32&)  d128[0] = 0;			break;
		case 8:  (u64&)  d128[0] = 0;			break;
		case 16: i_movups( d128, zerox );		break;

		case 6: {
			(u32&)  d128[0] = 0;
			((u16*)d128)[2] = 0;
		} break;

		case 10: {
			(u64&)  d128[0] = 0;
			((u16*)d128)[4] = 0;
		} break;

		case 12: {
			(u64&)  d128[0] = 0;
			((u32*)d128)[2] = 0;
		} break;

		case 20: {
			i_movups( d128, zerox );
			((u32&)d128[1]) = 0;
		} break;

		case 24: {
			i_movups( d128, zerox );
			((u64&)d128[1]) = 0;
		} break;

		case 32:  {
			i_movups( d128+0, zerox );
			i_movups( d128+1, zerox );
		} break;

		case 36:  {
			i_movups( d128+0, zerox );
			i_movups( d128+1, zerox );
			((u32&)d128[2]) = 0;
		} break;

		case 40:  {
			i_movups( d128+0, zerox );
			i_movups( d128+1, zerox );
			((u64&)d128[2]) = 0;
		} break;

		case 44:  {
			i_movups( d128+0, zerox );
			i_movups( d128+1, zerox );
			((u64&) d128[2])	= 0;
			((u32*)&d128[2])[2] = 0;
		} break;

		case 48:  {
			i_movups( d128+0, zerox );
			i_movups( d128+1, zerox );
			i_movups( d128+2, zerox );
		} break;

		case 64:  {
			i_movups( d128+0, zerox );
			i_movups( d128+1, zerox );
			i_movups( d128+2, zerox );
			i_movups( d128+3, zerox );
		} break;

		case 80:  {
			i_movups( d128+0, zerox );
			i_movups( d128+1, zerox );
			i_movups( d128+2, zerox );
			i_movups( d128+3, zerox );
			i_movups( d128+4, zerox );
		} break;

		default:
			if   (!(t_size & 7))		__stosq((u64*)		dest, 0, t_size / 8);
			elif (!(t_size & 3)) 		__stosd((_stos_s32) dest, 0, t_size / 4);
			elif (!(t_size & 1))		__stosw((u16*)		dest, 0, t_size / 2);
			else						__stosb((u8*)		dest, 0, t_size    );
		break;
	}
}

template< typename T >
inline __ai void xMemZero( T& dest )
{
	static_assert(__is_trivially_copyable(T), "This is not a trivially-copyable object!");
	static_assert(!std::is_pointer<T>::value, "xMemZero of pointer value.  Use var == nullptr to explicity zero pointer variables." );
	_internal_memzero<sizeof(T)>(&dest);
}

template< typename T, size_t _size >
inline __ai void xMemZero( T (*&dest)[_size] )
{
//	memset(dest, 0, _size * sizeof(T));
	_internal_memzero<_size * sizeof(T)>(&dest);
}

template< u8 data, typename T >
inline __ai void xMemSet( T& dest )
{
	static_assert(__is_trivially_copyable(T), "This is not a trivially-copyable object!");
	static_assert(!std::is_pointer<T>::value, "xMemSet of pointer value.  Did you mean to memset the buffer pointed to instead?" );
	memset(&dest, data, sizeof(T));
}

template< u8 data, typename T, size_t _size >
inline __ai void xMemSet( T (*&dest)[_size] )
{
	memset(dest, data, _size * sizeof(T));
}

inline __ai int xMemCmp( const void* d0, const void* d1, uint len )
{
	return memcmp(d0, d1, len);
}

#ifdef _MSC_VER
inline __ai s64 strtoll(const char * _Str, char ** _EndPtr, int _Radix )
{
	return _strtoi64( _Str, _EndPtr, _Radix );
}
#endif

inline __ai bool i_BitScanReverse( u32& result, u32 src )
{
#ifdef _MSC_VER
	return !!_BitScanReverse( (unsigned long*)&result, src );
#else
	return _BitScanReverse( &result, src );
#endif
}

inline __ai bool i_BitScanReverse( u64& result, u64 src )
{
#ifdef _MSC_VER
	return !!_BitScanReverse64( (unsigned long*)&result, src );
#else
	return _BitScanReverse64( &result, src );
#endif
}

inline __ai bool i_BitScanForward( u32& result, u32 src )
{
#ifdef _MSC_VER
	return !!_BitScanForward( (unsigned long*)&result, src );
#else
	return _BitScanForward( &result, src );
#endif
}

inline __ai bool i_BitScanForward( u64& result, u64 src )
{
#ifdef _MSC_VER
	return !!_BitScanForward64( (unsigned long*)&result, src );
#else
	return _BitScanForward64( &result, src );
#endif
}


extern xString DecodeBitField(u32 bits, const char* pLegend[], u32 numEntries);
extern xString xFixFilenameForPlatform( const xString& src );

extern void Host_RemoveFolder( const char* remFolder);
extern char xConvertBuildTargetToInt();
