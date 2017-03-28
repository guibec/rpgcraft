#pragma once

#include "x-types.h"


// non MSVC and an old MSVC don't have _mm_undefined_*.
// (gsgdb needs to be built with vs2010, because gtkmm libraries were with them...)
// ORBIS SDK4.00: '_mm_undefined_*' supported in internal/full SDK, but is still missing from
//    the public SDK (sdk-approved) as of SQA6, due to it using an older version of CLANG
//    (3.6 or 3.7).  So I've changed the ifdef to check CLANG version rather than SDK version.
//    With luck this will even work on linux clang "as intended." --jstine

#if !defined(HAS_MM_UNDEFINED)
#	if TARGET_ORBIS
#		if ((__clang_major__ >= 4) || (__clang_major__ == 3 && __clang_minor__ >= 8))
#			define HAS_MM_UNDEFINED		1
#		endif
#	elif defined(_MSC_VER) && (_MSC_VER > 1600)
#		define HAS_MM_UNDEFINED			1
#	endif
#endif

#if !defined(HAS_MM_UNDEFINED)
#	define HAS_MM_UNDEFINED				0
#endif

#if !HAS_MM_UNDEFINED
static inline __ai __m128 _mm_undefined_ps (void)
{
  __m128 Y = Y;
  return Y;
}

static inline __ai __m128i _mm_undefined_si128 (void)
{
  __m128i Y = Y;
  return Y;
}

static inline __ai __m128d _mm_undefined_pd (void)
{
  __m128d Y = Y;
  return Y;
}
#endif

#include "sse_intrinsics.h"

#define SPLAT_8x16(val)  { (val), (val), (val), (val), (val), (val), (val), (val) }
#define SPLAT_4x32(val)  { (val), (val), (val), (val) }
#define SPLAT_2x64(val)  { (val), (val) }

// --------------------------------------------------------------------------------------
// i_any_value(xmm_reg)
//   suppresses runtime errors relating to xmm initializtion.  Often an xmm reg's current
//   contents are not important to an operation (either because the operation is creating
//   an all-zero or all-1 style mask, or because the upper portion of the reg is ignored
//   due to scalar operation via xmm registers).
//
// UPDATE:  Clang 3.4 and GCC will optimize away the instruction that uses 'reg' in some
//   cases, which is really unwanted behavior.

#if TARGET_MSW && TARGET_DEBUG
#	define i_any_value(reg)		i_pxor(reg)
#else
#	define i_any_value(reg)		i_pxor(reg)
#endif
// --------------------------------------------------------------------------------------


inline __ai void Copy128_Unaligned( void* dest, const void* src )
{
	__m128 tmp;
	i_movups( tmp, (float*)src );
	i_movups( (float*)dest, tmp );
}

inline __ai void Copy128( void* dest, const void* src )
{
	__m128 tmp;
	i_movaps( tmp, (float*)src );
	i_movaps( (float*)dest, tmp );
}

// Non-temporal copies always assume unaligned source.
inline __ai void Copy128_NT( void* dest, const void* src )
{
	__m128 tmp;
	i_movups ( tmp, (float*)src );
	i_movntps( (float*)dest, tmp );
}

inline __ai void ZeroQWC( void* dest )
{
	i_movaps( (float*)dest, _mm_setzero_ps() );
}

// ----------------------------------------------------------------------------
//  u128  (union)
// ----------------------------------------------------------------------------
// The most over-bloated and awesome type definition ever contrived.  One u128
// to rule them all!
//
// Quick notes:
//   Prefer using the explicit .d0, .d1, .b0-b15, etc over using the array verisons
//   .d[0], b[15], etc.  This way endian swapping can be performed in-union instead of
//   having to issue endian-swapped versions of all assignment code.  Arrayized versions
//   are included for looping; in whcih case loops will need to be adjusted to handle
//   endianness as-needed.
//
union u128
{
	//
	// MIPS-centric naming convention
	//
	// byte, halfword, word, doubleword, and quadword.  It is designed so that signed and
	// unsigned doublewords (64 bits) can be accessed easily witout nested member
	// denominations (see 'ud' and 'sd').
	//

	__m128	qf;				// quadword packed floats
	__m128i	qi;				// quadword packed integers
	__m128d	qd;				// quadword packed doubles

	struct
	{
		u64 lo;
		u64 hi;
	};

	u64		d[2];			// unsigned doubleword (64 bits)
	u32		w[4];			// unsigned word (32 bits)
	u16		h[8];			// unsigned halfword (16 bits)
	u8		b[16];			// unsigned byte (8 bits)

	s64		sd[2];			// signed doubleword (64 bits)
	s32		sw[4];			// signed word (32 bits)
	s16		sh[8];			// signed halfword (16 bits)
	s8		sb[16];			// signed byte (8 bits)

	//
	// Vector-style accessors:
	//

	struct
	{
		float xf, yf, zf, wf;
	};

	struct
	{
		s32 xi, yi, zi, wi;
	};

	float f[4];

	//
	// Why require the use of brackets?
	// I like these even better:
	//

	struct
	{
		float f0, f1;
		float f2, f3;
	};

	struct
	{
		u64 d0;
		u64 d1;
	};

	struct
	{
		s64 sd0;
		s64 sd1;
	};

	struct
	{
		u32	w0, w1;
		u32	w2, w3;
	};

	struct
	{
		s32	sw0, sw1;
		s32	sw2, sw3;
	};

	struct
	{
		u16	h0, h1, h2, h3;
		u16	h4, h5, h6, h7;
	};

	struct
	{
		s16	sh0, sh1, sh2, sh3;
		s16	sh4, sh5, sh6, sh7;
	};

	struct
	{
		u8	b0,   b1,   b2,   b3;
		u8	b4,   b5,   b6,   b7;
		u8	b8,   b9,   b10,  b11;
		u8	b12,  b13,  b14,  b15;
	};

	struct
	{
		s8	sb0,   sb1,   sb2,   sb3;
		s8	sb4,   sb5,   sb6,   sb7;
		s8	sb8,   sb9,   sb10,  sb11;
		s8	sb12,  sb13,  sb14,  sb15;
	};

	//
	// Explicit conversion from u64. Zero-extends the source through 128 bits.
	//
	static __ai u128 from64( u64 src )
	{
		u128 result;
		i_movsd_zx( result, (double*)&src );
		return result;
	}

	static __ai u128 splat64( u64 src )
	{
		u128 result;
		i_splat_pd(result, (double&)src);
		return result;
	}

	//
	// Explicit conversion from u32. Zero-extends the source through 128 bits.
	//
	static __ai u128 from32( u32 src )
	{
		u128 result;
		i_movss_zx( result, (float*)&src );
		return result;
	}

	static __ai u128 splat32( s32 src )
	{
		u128 result;
		i_splat32(result, src);
		return result;
	}

	static __ai u128 fromQuad( const __m128d& src )
	{
		u128 result;
		result.qd = src;
		return result;
	}

	static __ai u128 fromQuad( const __m128i& src )
	{
		u128 result;
		result.qi = src;
		return result;
	}

	static __ai u128 fromQuad( const __m128& src )
	{
		u128 result;
		result.qf = src;
		return result;
	}

	__ai operator __m128&		()			{ return qf; }
	__ai operator u32&			()			{ return w0; }
	__ai operator u16&			()			{ return h0; }
	__ai operator u8&			()			{ return b0; }

	__ai operator const __m128&	() const	{ return qf; }
	__ai operator const u32&	() const	{ return w0; }
	__ai operator const u16&	() const	{ return h0; }
	__ai operator const u8&		() const	{ return b0; }

	__ai bool operator==( const u128& right ) const
	{
		//return (lo == right.lo) && (hi == right.hi);

		u128 result;
		i_pcmpeqd	(result, qf, right.qf);
		i_pxor		(result, result, u128().SetFFs());
		return i_ptestz(result);
	}

	__ai bool operator!=( const u128& right ) const
	{
		//return (lo != right.lo) || (hi != right.hi);

		u128 result;
		i_pcmpeqd	(result, qf, right.qf);
		i_pxor		(result, result, u128().SetFFs());
		return !i_ptestz(result);
	}

	__ai u128& operator=( const __m128& src )
	{
		qf = src;
		return *this;
	}

	__ai u128& operator=( const __m128i& src )
	{
		qi = src;
		return *this;
	}

	__ai u128 operator&( const u128& src ) const
	{
		u128 result;
		i_pand( result, qf, src.qf );
		return result;
	}

	__ai u128 operator|( const u128& src ) const
	{
		u128 result;
		i_por( result, qf, src.qf );
		return result;
	}

	__ai u128 operator^( const u128& src ) const
	{
		u128 result;
		i_pxor( result, qf, src.qf );
		return result;
	}

	__ai u128 operator~() const
	{
		__m128	neg1;
		u128	result;
		i_pcmpeq	(neg1);
		i_pxor		(result, neg1,	qf);
		return result;
	}

	__ai u128& operator&=( const u128& src )
	{
		i_pand( qf, qf, src.qf );
		return *this;
	}

	__ai u128& operator|=( const u128& src )
	{
		i_por( qf, qf, src.qf );
		return *this;
	}

	__ai u128& operator^=( const u128& src )
	{
		i_pxor( qf, qf, src.qf );
		return *this;
	}

	__ai u128& SetZero()
	{
		i_pxor( qf );
		return* this;
	}

	__ai u128& SetFFs()
	{
		i_pcmpeq(qf);
		return* this;
	}

} __aligned(16);


union u128x2 {
	struct {
		s64		sd[2];
	};
	u128	q;

	__ai operator __m128&			()			{ return q.qf;	}
	__ai operator u128&				()			{ return q;		}

	__ai operator const __m128&		() const	{ return q.qf;	}
	__ai operator const u128&		() const	{ return q;		}

	__ai bool operator==( const u128& right ) const	{ return q == right; }
	__ai bool operator!=( const u128& right ) const	{ return q != right; }
};

union u128x4 {
	struct {
		u32		w[4];
	};
	u128	q;

	__ai operator __m128&			()			{ return q.qf;	}
	__ai operator u128&				()			{ return q;		}

	__ai operator const __m128&		() const	{ return q.qf;	}
	__ai operator const u128&		() const	{ return q;		}

	__ai bool operator==( const u128& right ) const	{ return q == right; }
	__ai bool operator!=( const u128& right ) const	{ return q != right; }
};

union u128x8 {
	struct {
		u16		h[8];
	};
	u128	q;

	__ai operator __m128&			()			{ return q.qf;	}
	__ai operator u128&				()			{ return q;		}

	__ai operator const __m128&		() const	{ return q.qf;	}
	__ai operator const u128&		() const	{ return q;		}

	__ai bool operator==( const u128& right ) const	{ return q == right; }
	__ai bool operator!=( const u128& right ) const	{ return q != right; }
};

union f128x4 {
	struct {
		float	f[4];
	};
	u128	q;

	__ai operator __m128&			()			{ return q.qf;	}
	__ai operator u128&				()			{ return q;		}

	__ai operator const __m128&		() const	{ return q.qf;	}
	__ai operator const u128&		() const	{ return q;		}

	__ai bool operator==( const u128& right ) const	{ return q == right; }
	__ai bool operator!=( const u128& right ) const	{ return q != right; }
};


static_assert( sizeof(u128) == 16, "A u128 is is not 128 bits long is scarcely a u128 at all!" );

extern xString xDataStr( const __m128& src );		// Requires also x-string.h included by programmer

// iterator to traverse just only 'bit-set' positions to avoid unnecessary huge looping.
// NOTE:
//		tzcnt(0) returns size of the operand
//		_mm_popcnt(0) returns 0
// TODO: u128_iter is quite slow due to get_next()... u64_iter is beter to use but it needs to have 2 loops...
struct u128_iter
{
	mutable s64		ntz;
	mutable u128	_current;
	void get_next() const {
#if TARGET_ORBIS
		s64 lntz, hntz;
		__asm__("tzcnt %[curL], %[lntz];"
				"tzcnt %[curH], %[hntz];"
				"movq  %[curL],%%rax;"
				"movq  %[curH],%%rdx;"				
				"addq  $-1, %[curL];"
				"adcq  $-1, %[curH];"
				"andq  %%rax, %[curL];"
				"andq  %%rdx, %[curH];"	
				: [lntz] "=r" (lntz),
				  [hntz] "=r" (hntz),
				  [curL] "+&r"(_current.sd[0]),
				  [curH] "+&r"(_current.sd[1])
				:: "rax", "rdx");
				  
		ntz = (lntz == 64) ? hntz + 64 : lntz;
#else
		if (_current.sd[0] != 0) {
			ntz = i_popcnt((_current.sd[0] & (-_current.sd[0])) - 1);	// number of trailing zero
			_current.sd[0] &= _current.sd[0] - 1;						// clear NTZ bit
		} else if (_current.sd[1] != 0) {
			ntz = i_popcnt((_current.sd[1] & (-_current.sd[1])) - 1);
			ntz += 64;
			_current.sd[1] &= _current.sd[1] - 1;
		} else
			ntz = NTZ_0_VAL;
#endif
	}
	u128_iter& operator++()  {
		get_next();
		return *this;
	}
	const u128_iter& operator++() const {		// how does a const++ make any sense?  C++ iterators, for shame! -_-  --jstine
		get_next();
		return *this;
	}

	u128_iter operator+(int incr) const {
		u128_iter copy = *this;
		copy.get_next();
		return copy;
	}

	bool operator==( const u128_iter& rhs ) const { return ntz == rhs.ntz; }
	bool operator==(	   u128_iter& rhs ) const { return ntz == rhs.ntz; }
	bool operator!=( const u128_iter& rhs ) const { return ntz != rhs.ntz; }
	bool operator!=( 	   u128_iter& rhs ) const { return ntz != rhs.ntz; }
	int operator*() const {	return ntz; }

	static u128_iter begin(const u128& bits) { return u128_iter(bits); }
	static u128_iter end  (const u128& bits) { return u128_iter(NTZ_0_VAL); }						// bits doesn't actually matter for "end" iterator

protected:
	enum { NTZ_0_VAL = 128 };
	u128_iter(const u128& bits) 			{ _current = bits;					get_next(); }
	u128_iter(const u128& bits, int val)	{ _current = bits;					ntz = val; }
	u128_iter(int val)						{ _current = _mm_undefined_ps();	ntz = val; }		// bits doesn't actually matter for "end" iterator
};

struct u64_iter
{
	mutable s64	ntz;
	mutable s64	_current;
	void get_next() const {
#if TARGET_ORBIS
		__asm__("tzcnt %[val], %[result]" : [result] "=r"(ntz) : [val] "r"(_current));
#else
		if (_current == 0)
			ntz = 64;
		else {
			ntz = i_popcnt((_current & (-_current)) - 1);
		}
#endif
		_current &= _current - 1;
	}
	u64_iter& operator++() {
		get_next();
		return *this;
	}
	const u64_iter& operator++() const {
		get_next();
		return *this;
	}
	bool operator==( const u128_iter& rhs ) const { return ntz == rhs.ntz; }
	bool operator==(	   u128_iter& rhs ) const { return ntz == rhs.ntz; }
	bool operator!=( const u128_iter& rhs ) const { return ntz != rhs.ntz; }
	bool operator!=( 	   u128_iter& rhs ) const { return ntz != rhs.ntz; }
	int operator*() const {	return ntz; }

	static u64_iter begin(const u64& bits) { return u64_iter(bits); }
	static u64_iter end  (const u64& bits)	{ return u64_iter(bits, NTZ_0_VAL); }
protected:
	enum { NTZ_0_VAL = 64 };
	u64_iter(const u64& bits)				{ _current = bits; get_next(); }
	u64_iter(const u64& bits, int val)		{ _current = bits; ntz = val; }
};
