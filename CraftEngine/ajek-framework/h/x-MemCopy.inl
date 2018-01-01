#pragma once

#include "x-simd.h"
#include "x-stdlib.h"

template< size_t len >
inline __xi void xMemCopySmall( void* dest, const void* src )
{
    static_assert(len != 0, "Memcopy length is null!");
    switch(len) {
        case 1:     *(u8*)dest  = *(u8*)src;            break;
        case 2:     *(u16*)dest = *(u16*)src;           break;
        case 4:     *(u32*)dest = *(u32*)src;           break;
        case 8:     *(u64*)dest = *(u64*)src;           break;

        case 12:    *(u64*)dest     = *(u64*)src;
                    *((u32*)dest+2) = *((u32*)src+2);   break;

        case 16:    Copy128_Unaligned(dest, src);       break;
        case 20:    Copy128_Unaligned(dest, src);
                    *((u32*)dest+4) = *((u32*)src+4);   break;
        case 24:    Copy128_Unaligned(dest, src);
                    *((u64*)dest+2) = *((u64*)src+2);   break;
        case 28:    Copy128_Unaligned(dest, src);
                    *((u64*)dest+2) = *((u64*)src+2);
                    *((u32*)dest+6) = *((u32*)src+6);   break;

        case 32:    Copy128_Unaligned(dest, src);
                    Copy128_Unaligned((u8*)dest+16, (u8*)src+16); break;

        case 48:    Copy128_Unaligned(dest, src);
                    Copy128_Unaligned((u8*)dest+16, (u8*)src+16);
                    Copy128_Unaligned((u8*)dest+32, (u8*)src+32); break;

        case 64:    Copy128_Unaligned(dest, src);
                    Copy128_Unaligned((u8*)dest+16, (u8*)src+16);
                    Copy128_Unaligned((u8*)dest+32, (u8*)src+32);
                    Copy128_Unaligned((u8*)dest+48, (u8*)src+48); break;

        default:
            xMemCopy( dest, src, len );
        break;
    }
}

// --------------------------------------------------------------------------------------
// Copies memory from src to dest in whatever fashion mighty Olympus deems most awesome
// and irrefutably efficient.  See inside for implementation details!
//
// Note that this still uses temporal (cached) copy mechanisms.  Copies that are queueing
// data into buffer for use by a separate thread should use xMemCopyQwcNT.
//
inline __xi void xMemCopy( void* dest, const void* src, uint len )
{
    //bug_on_qa(!len);

    //
    // i3/i5/i7 specific:
    //  movsd/movsq are usually the best options for moving anything more than 256 bytes of data.
    //  They're also uop cache friendly.  I love it when it's easy.
    //
    // AMD Jaguar update:
    //  movsq is still pretty good but the threshold is a bit higher, maybe around 1kb.
    //  For copies frequently below 1kb, it is probably better to use xmm copy.
    //

    __movsb( (u8*)dest, (u8*)src, len );
}

// --------------------------------------------------------------------------------------
inline __xi void xMemCopy32( void* dest, const void* src, uint len32 )
{
    __movsd( (unsigned long*)dest, (unsigned long*)src, len32 );    // typecast for intel/msvc prototype
}

// --------------------------------------------------------------------------------------
inline __xi void xMemCopy64( void* dest, const void* src, uint len64 )
{
    __movsq( (u64*)dest, (u64*)src, len64 );    // typecast for intel/msvc prototype
}

// --------------------------------------------------------------------------------------
inline __xi void xMemCopyConst32( void* dest, const void* src, uint len32 )
{
            u64* dest64 = (u64*)dest;
    const   u64* src64  = (u64*)src;
    const   u64* destEnd = (u64*)dest + len32 / 2;

    for (; dest64 < destEnd; ++dest64, ++src64 ) {

#if TARGET_MSW
        dest64[0] = src64[0];
#else
        // Oh Clever CLANG -- it inlines some implementation of memcpy() automatically
        // for us.  Except, we don't WANT memcpy().  This memory copy is meant for short
        // copies averaging 128-256 bytes.  For these kind of copies it pays to have as
        // little entry/exit overhead as possible.  Thus I've used GAS to disable the
        // magic memcpy() insertion mess:
        //
        // (this is especially important to VIF/VU UNPACK processing on the EE thread)

        u64 tmp;
        __asm__("movq %[src], %[tmp];" 
            :   [tmp] "=r" (tmp)
            :   [src] "m" (src64[0])
            :
        );
        __asm__("movq %[tmp], %[dest];"
            :   [dest]  "+m" (dest64[0])
            :   [tmp] "r" (tmp)
            : "memory"
        );
#endif
    }

    if (len32 & 1) {
        (u32&)dest64[0] = (u32&)src64[0];
    }
}

inline __xi void xMemCopy32_NT( void* dest, const void* src, uint len32 )
{
    u32 *dest32 = (u32*)dest;
    const u32 *src32 = (u32*)src;
    const u32 *destEnd32 = dest32 + len32;
    for( ; dest32 <destEnd32; ++dest32, ++src32)
    {
        //*dest32 = *src32;
        i_movnti( dest32,*src32) ;
    }
}
// --------------------------------------------------------------------------------------
// Non-temporal copies always assume unaligned source.  Destination must always be aligned.
inline __xi void xMemCopyShortQwc_NT( void* dest, const void* src, uint lenQwc )
{
            __m128* dest128 = (__m128*)dest;
    const   __m128* src128  = (__m128*)src;
    //const __m128* destEnd = dest128 + lenQwc;

    // note: must use ++i (even tho i is unused) to hint to clang about loop unrolling.
    for (uint i=0 ; i<lenQwc; ++i, ++dest128, ++src128 )
    {
        Copy128_NT (dest128, src128);
    }
}

// --------------------------------------------------------------------------------------
// tailored for use of copies of 32 QW or less.
//
inline __xi void xMemCopyShortQwc( void* dest, const void* src, uint lenQwc )
{
            __m128* dest128 = (__m128*)dest;
    const   __m128* src128  = (__m128*)src;
    //const __m128* destEnd = dest128 + lenQwc;

    // note: must use ++i (even tho i is unused) to hint to clang about loop unrolling.
    for (uint i=0 ; i<lenQwc; ++i, ++dest128, ++src128 )
    {
        Copy128_Unaligned (dest128, src128);
    }
}

// --------------------------------------------------------------------------------------
// handy routine for copying quadwords (128bits) around.  Length parameter is in qwc units,
// so no div/shifts or remainder checking needed.
//
inline __xi void xMemCopyQwc( void* dest, const void* src, uint lenQwc )
{
#if TARGET_x32
    __movsd( (unsigned long*)dest, (unsigned long*)src, lenQwc * 4 );
#else
    __movsq( (u64*)dest, (u64*)src, lenQwc * 2 );
#endif
}

// --------------------------------------------------------------------------------------
// Non-temporal memcpy implementation.
// Needs benchmarking.  It's entirely possible that movsq does non-temporal copies
// internally for large copies (though that also may be specific to Intel archs). --jstine
//
inline __xi void xMemCopyQwc_NT( void* dest, const void* src, uint lenQwc )
{
            __m128      r1, r2, r3, r4;
    const   __m128i*    srcPtr  = (__m128i*)src;
            __m128i*    destPtr = (__m128i*)dest;

    // the non-temporal cache lines are 64-bytes long so, in theory, unrolling the loop
    // like so should maximize the cpu's pipeline.

    uint len64      = lenQwc / 4;
    uint remainder  = lenQwc & 3;
    while( len64 )
    {
        i_movups( r1, srcPtr+0 );
        i_movups( r2, srcPtr+1 );
        i_movups( r3, srcPtr+2 );
        i_movups( r4, srcPtr+3 );
        i_movntps( destPtr+0, r1 );
        i_movntps( destPtr+1, r2 );
        i_movntps( destPtr+2, r3 );
        i_movntps( destPtr+3, r4 );
        srcPtr  += 4;
        destPtr += 4;
        --len64;
    }

    switch( remainder )
    {
        case 3: i_movups( r1, srcPtr+2 );
                i_movntps( destPtr+2, r1 );

        case 2: i_movups( r1, srcPtr+1 );
                i_movntps( destPtr+1, r1 );

        case 1: i_movups( r1, srcPtr+0 );
                i_movntps( destPtr+0, r1 );

        case 0:
        break;

        default: __unreachable();
    }

    // Note: mfence should not be needed if other systems are implemented correctly.
    // Threads should use atomic exchanges (which implicitly include mfence) to signal
    // other threads to use data.  And systems that may re-read non-temporal data back
    // shortly after writing should mfence before reading. (and they shouldn't do that
    // anyway -- it's slow!)
    //
    //_mm_mfence();
}

// --------------------------------------------------------------------------------------
template< typename T, typename T2 >
inline __ai void xObjCopy( T& dest, const T2& src )
{
    static_assert(__is_trivially_copyable(T),   "This is not a trivially-copyable object!");
    static_assert(__is_trivially_copyable(T2),  "This is not a trivially-copyable object!");
    static_assert( sizeof(T) == sizeof(T2),     "Objects are not of the same size!");

    // Size is known so a fancy copy can be used -- compiler will factor out most
    // of the conditionals.

    if (sizeof(T) > 384) {
        xMemCopy(&dest, &src, sizeof(T));
        return;
    }

    static const size_t qwcCount =  sizeof(T)       / 16;
    static const size_t wcCount  = (sizeof(T) & 15) / 4;
    static const size_t bcCount  =  sizeof(T) & 3;

    static_assert(((qwcCount*16) + (wcCount*4) + bcCount) == sizeof(T), "Programmer fail: Size count mismatch in xObjCopy.");

            __m128* dest128 = (__m128*)&dest;
    const   __m128* src128  = (__m128*)&src;

    if (qwcCount) xMemCopyShortQwc(&dest, &src, qwcCount);
    if (wcCount)  xMemCopyConst32 (dest128 + qwcCount, src128 + qwcCount, wcCount);

            u8* dest8   = (u8*)(dest128 + qwcCount) + wcCount;
    const   u8* src8    = (u8*)(src128  + qwcCount) + wcCount;

    if (bcCount > 0) dest8[0] = src8[0];
    if (bcCount > 1) dest8[1] = src8[1];
    if (bcCount > 2) dest8[2] = src8[2];
}

template< typename T, typename T2 >
inline __ai int xObjCompare( const T& left, const T2& right )
{
    static_assert(__is_trivially_copyable(T),   "This is not a trivially-copyable object!");
    static_assert(__is_trivially_copyable(T2),  "This is not a trivially-copyable object!");
    static_assert( sizeof(T) == sizeof(T2),     "Objects are not of the same size!");

    return xMemCmp(&left, &right, sizeof(T));
}

// --------------------------------------------------------------------------------------
//  xMemCopy_WrappedDest / xMemCopy_WrappedSrc
// --------------------------------------------------------------------------------------
// Mostly useful burst for DMA copies to/from SPR and VU memory.
// Remarks:
//   the dest/src start position is passed BY REFERENCE, and is updated in-place with
//   wrapping applied.  This is typically very useful for updating internal position info
//   and avoiding redundant wrapping logic.
//
// Performance notes:
//   # Yes these are much faster than using unrolled quad-for-quad copies for everything
//     except perhaps copies under 16 quads in length.
//   # dest/src Size should be a maskable binary number (8, 16, 32, etc) for best performance.
//   # all modern compilers will inline these 100% reliably, and use "AND destSize-1"
//     instead of "MODULO destsize".
//
extern void xMemCopyQwc_WrappedDest (u128* destBase, const u128* src, uint& destStartQwc, uint lenQwc, uint destSizeQwc);
extern void xMemCopyQwc_WrappedSrc  (u128* dest, const u128* srcBase, uint& srcStartQwc, uint lenQwc, uint srcSizeQwc);
