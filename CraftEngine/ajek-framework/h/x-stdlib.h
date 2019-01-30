
#pragma once

#include "x-types.h"
#include "x-simd.h"
#include "x-stl.h"
#include "x-unipath.h"

#include <cstring>      // needed for memset
#include <type_traits>

#define serialization_assert(T) \
    static_assert(xIs_trivially_copyable(T), "Cannot serialize non-POD object.")

#if TARGET_LINUX
#   define printf_s         printf
#   define fprintf_s        fprintf
#   define vfprintf_s       vfprintf

#   define fopen_s(hptr, fpath, mode) \
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

class FunctHashAlignedPtr {
public:
    __xi size_t operator()(const void* const& input) const {
        static_assert(sizeof(input) <= sizeof(size_t), "Unsupported pointer hashing situation. A _real_ hash will be required here!");
        return (size_t)(sptr(input) >> 4);
    }
};

template<typename T>
void placement_delete(T* ptr)   { (ptr)->~T(); xFree(ptr); }

extern void         xStrCopy                (char* dest, size_t destLen, const char* src);
extern void         xStrnCopy               (char* dest, size_t destLen, const char* src, size_t srcLen);

extern void         xMemCopy                (void* dest, const void* src, uint len);
extern void         xMemCopy32              (void* dest, const void* src, uint len32);
extern void         xMemCopyQwc             (void* dest, const void* src, uint lenQwc);
extern void         xMemCopyQwc_WrappedDest (u128* destBase,    const u128* src,        uint& destStartQwc, uint lenQwc, uint destSizeQwc);
extern void         xMemCopyQwc_WrappedSrc  (u128* dest,        const u128* srcBase,    uint& srcStartQwc,  uint lenQwc, uint srcSizeQwc);

extern void         xMemCopyShortQwc        (void* dest, const void* src, uint lenQwc);
extern void         xMemCopyShortQwc_NT     (void* dest, const void* src, uint lenQwc);

extern bool         xEnvironExists          (const xString& varname);
extern xString      xEnvironGet             (const xString& varname);
extern void         xEnvironSet             (const xString& varname, const xString& value, bool overwrite=1);

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
void xStrnCopyT(char (&dest)[destSize], const char* src, size_t srcLen)
{
    xStrnCopy(dest, destSize, src, srcLen);
}

template< int destSize >
void xStrCopyT(char (&dest)[destSize], const char* src)
{
    xStrCopy(dest, destSize, src);
}


// ======================================================================================
#ifdef _MSC_VER
    typedef unsigned long*  _stos_s32;
#else
    typedef u32*            _stos_s32;
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
        case 1:  (u8&)   d128[0] = 0;           break;
        case 2:  (u16&)  d128[0] = 0;           break;
        case 4:  (u32&)  d128[0] = 0;           break;
        case 8:  (u64&)  d128[0] = 0;           break;
        case 16: i_movups( d128, zerox );       break;

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
            ((u64&) d128[2])    = 0;
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
            if   (!(t_size & 7))        __stosq((u64*)      dest, 0, t_size / 8);
            elif (!(t_size & 3))        __stosd((_stos_s32) dest, 0, t_size / 4);
            elif (!(t_size & 1))        __stosw((u16*)      dest, 0, t_size / 2);
            else                        __stosb((u8*)       dest, 0, t_size    );
        break;
    }
}

template< typename T >
inline __ai void xMemZero( T& dest )
{
    //static_assert(std::is_trivially_default_constructible<T>::value, "Unsafe use of memset on non-trivial object type.");
    static_assert(xIs_trivially_copyable(T), "This is not a trivially-copyable object!");
    static_assert(!std::is_pointer<T>::value, "xMemZero of pointer value.  Use var == nullptr to explicity zero pointer variables." );
    _internal_memzero<sizeof(T)>(&dest);
}

template< typename T, size_t _size >
inline __ai void xMemZero( T (*&dest)[_size] )
{
//  memset(dest, 0, _size * sizeof(T));
    _internal_memzero<_size * sizeof(T)>(&dest);
}

template< u8 data, typename T >
inline __ai void xMemSet( T& dest )
{
    static_assert(xIs_trivially_copyable(T), "This is not a trivially-copyable object!");
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

// not actually sure when strtoll was added to MSVC, somewhere duing VS 2017 lifespan.
#if defined(_MSC_VER) && (_MSC_VER < 1910)
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

extern void Host_RemoveFolder( const char* remFolder);
extern char xConvertBuildTargetToInt();
