
#pragma once

#include "x-types.h"

// An annoying layer file needed because MSVC's Interlocked functions pre-dated strong
// typing.  They use 'long' to reference 32 bit values, which GCC finds unacceptable
// (and for good reason).  --jstine

#ifdef _MSC_VER
	typedef long volatile* _interlocked_s32;
#else
	typedef s32 volatile* _interlocked_s32;
#endif

inline __ai s32 AtomicInc( volatile s32& val )
{
	return _InterlockedIncrement( (_interlocked_s32) &val );
}

inline __ai s64 AtomicInc( volatile s64& val )
{
	return _InterlockedIncrement64( (s64 volatile*) &val );
}

inline __ai s32 AtomicDec( volatile s32& val )
{
	return _InterlockedDecrement( (_interlocked_s32) &val );
}

inline __ai s64 AtomicDec( volatile s64& val )
{
	return _InterlockedDecrement64( (s64 volatile*) &val );
}


inline __ai s32 AtomicExchange( volatile s32& dest, s32 src )
{
	return _InterlockedExchange( (_interlocked_s32) &dest, src );
}

inline __ai s64 AtomicExchange( volatile s64& dest, s64 src )
{
	return _InterlockedExchange64( (s64 volatile*) &dest, src );
}

inline __ai s32 AtomicExchangeAdd( volatile s32& src, s32 amount )
{
	return _InterlockedExchangeAdd( (_interlocked_s32) &src, (long)amount );
}

inline __ai s64 AtomicExchangeAdd( volatile s64& src, s64 amount )
{
	return _InterlockedExchangeAdd64( (s64 volatile*) &src, amount );
}

inline __ai s32 AtomicCompareExchange( volatile s32& dest, s32 exchange, s32 comparand )
{
	return _InterlockedCompareExchange( (_interlocked_s32) &dest, exchange, comparand );
}

inline __ai s64 AtomicCompareExchange( volatile s64& dest, s64 exchange, s64 comparand )
{
	return _InterlockedCompareExchange64( (s64 volatile*) &dest, exchange, comparand );
}

template< typename T >
inline __ai T AtomicCompareExchangeEnum( volatile T& dest, T exchange, T comparand )
{
	static_assert( sizeof(T) == 4, "Sizeof enum type must be u32/s32!" );
	return (T)_InterlockedCompareExchange( (_interlocked_s32) &dest, (s32)exchange, (s32)comparand );
}
