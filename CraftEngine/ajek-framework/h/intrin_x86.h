
#pragma once

/*
	Compatibility <intrin_x86.h> header for GCC -- GCC equivalents of intrinsic
	Microsoft Visual C++ functions. Originally developed for the ReactOS
	(<http://www.reactos.org/>) and TinyKrnl (<http://www.tinykrnl.org/>)
	projects.

	Copyright (c) 2006 KJK::Hyperion <hackbunny@reactos.com>

	Permission is hereby granted, free of charge, to any person obtaining a
	copy of this software and associated documentation files (the "Software"),
	to deal in the Software without restriction, including without limitation
	the rights to use, copy, modify, merge, publish, distribute, sublicense,
	and/or sell copies of the Software, and to permit persons to whom the
	Software is furnished to do so, subject to the following conditions:

	The above copyright notice and this permission notice shall be included in
	all copies or substantial portions of the Software.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER_BitScanForward
	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
	FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
	DEALINGS IN THE SOFTWARE.
*/

#ifndef KJK_INTRIN_X86_H_
#define KJK_INTRIN_X86_H_

//
// Intel includes its own intrinsics as part of xmmintrin.h / immintrin.h, etc. --jstine
//

#if defined(__INTEL_COMPILER)
	// Contents of this file not needed.
	// Must check Intel first tho because it "cleverly" defines __GNUC__
#elif defined(__GNUC__) || defined(__clang__)

#include <x86intrin.h>
#include <stddef.h>
#include <stdint.h>


#if defined(__ORBIS__)
#	if TARGET_DEBUG
#		define INTRIN_INLINE
#		define INTRIN_ACCESSOR		inline __attribute__((always_inline))
#	else
#		define INTRIN_INLINE		inline __attribute__((always_inline))
#		define INTRIN_ACCESSOR		inline __attribute__((always_inline))
#	endif
#else
#	if TARGET_DEBUG
#		define INTRIN_INLINE
#		define INTRIN_ACCESSOR		__always_inline
#	else
#		define INTRIN_INLINE		__always_inline
#		define INTRIN_ACCESSOR		__always_inline
#	endif
#endif


//
// Intel provides a "simplified" setjmp that bypasses windows kernel invocation
// overhead, as _setjmpex.  On GCC it suffices to simply use setjmp --jstine
//
#define _setjmpex(jbuf) setjmp(jbuf)


/*
	FIXME: review all "memory" clobbers, add/remove to match Visual C++
	behavior: some "obvious" memory barriers are not present in the Visual C++
	implementation - e.g. __stosX; on the other hand, some memory barriers that
	*are* present could have been missed
*/

/*
	NOTE: this is a *compatibility* header. Some functions may look wrong at
	first, but they're only "as wrong" as they would be on Visual C++. Our
	priority is compatibility

	NOTE: unlike most people who write inline asm for GCC, I didn't pull the
	constraints and the uses of __volatile__ out of my... hat. Do not touch
	them. I hate cargo cult programming

	NOTE: be very careful with declaring "memory" clobbers. Some "obvious"
	barriers aren't there in Visual C++ (e.g. __stosX)

	NOTE: review all intrinsics with a return value, add/remove __volatile__
	where necessary. If an intrinsic whose value is ignored generates a no-op
	under Visual C++, __volatile__ must be omitted; if it always generates code
	(for example, if it has side effects), __volatile__ must be specified. GCC
	will only optimize out non-volatile asm blocks with outputs, so input-only
	blocks are safe. Oddities such as the non-volatile 'rdmsr' are intentional
	and follow Visual C++ behavior

	NOTE: on GCC 4.1.0, please use the __sync_* built-ins for barriers and
	atomic operations. Test the version like this:

	#if (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__) > 40100
		...

	Pay attention to the type of barrier. Make it match with what Visual C++
	would use in the same case
*/

#ifdef __cplusplus
extern "C" {
#endif

/*** Stack frame juggling ***/
#define _ReturnAddress() (__builtin_return_address(0))
#define _AddressOfReturnAddress() (&(((void **)(__builtin_frame_address(0)))[1]))
/* TODO: __getcallerseflags but how??? */

/* Maybe the same for x86? */
#ifdef __x86_64__
#	define _alloca(s) __builtin_alloca(s)
#endif

/*** Atomic operations ***/

#if (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__) > 40100
#define _ReadWriteBarrier() __sync_synchronize()
#else
INTRIN_ACCESSOR void _MemoryBarrier(void)
{
	__asm__ __volatile__("" : : : "memory");
}
#define _ReadWriteBarrier() _MemoryBarrier()
#endif

/* BUGBUG: GCC only supports full barriers */
#define _ReadBarrier _ReadWriteBarrier
#define _WriteBarrier _ReadWriteBarrier

#if (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__) > 40100

INTRIN_ACCESSOR s8 _InterlockedCompareExchange8(volatile s8 * const Destination, const s8 Exchange, const s8 Comperand)
{
	return __sync_val_compare_and_swap(Destination, Comperand, Exchange);
}

INTRIN_ACCESSOR s16 _InterlockedCompareExchange16(volatile s16 * const Destination, const s16 Exchange, const s16 Comperand)
{
	return __sync_val_compare_and_swap(Destination, Comperand, Exchange);
}

INTRIN_ACCESSOR s32 _InterlockedCompareExchange(volatile s32 * const Destination, const s32 Exchange, const s32 Comperand)
{
	return __sync_val_compare_and_swap(Destination, Comperand, Exchange);
}

INTRIN_ACCESSOR void * _InterlockedCompareExchangePointer(void * volatile * const Destination, void * const Exchange, void * const Comperand)
{
	return (void *)__sync_val_compare_and_swap(Destination, Comperand, Exchange);
}

INTRIN_ACCESSOR s32 _InterlockedExchange(volatile s32 * const Target, const s32 Value)
{
	/* NOTE: __sync_lock_test_and_set would be an acquire barrier, so we force a full barrier */
	__sync_synchronize();
	return __sync_lock_test_and_set(Target, Value);
}

#if defined(__x86_64__)
INTRIN_ACCESSOR s64 _InterlockedExchange64(volatile s64 * const Target, const s64 Value)
{
	/* NOTE: __sync_lock_test_and_set would be an acquire barrier, so we force a full barrier */
	__sync_synchronize();
	return __sync_lock_test_and_set(Target, Value);
}
#endif

INTRIN_ACCESSOR void * _InterlockedExchangePointer(void * volatile * const Target, void * const Value)
{
	/* NOTE: ditto */
	__sync_synchronize();
	return (void *)__sync_lock_test_and_set(Target, Value);
}

INTRIN_ACCESSOR s32 _InterlockedExchangeAdd16(volatile s16 * const Addend, const s16 Value)
{
	return __sync_fetch_and_add(Addend, Value);
}

INTRIN_ACCESSOR s32 _InterlockedExchangeAdd(volatile s32 * const Addend, const s32 Value)
{
	return __sync_fetch_and_add(Addend, Value);
}

#if defined(__x86_64__)
INTRIN_ACCESSOR s64 _InterlockedExchangeAdd64(volatile s64 * const Addend, const s64 Value)
{
	return __sync_fetch_and_add(Addend, Value);
}
#endif

INTRIN_ACCESSOR s8 _InterlockedAnd8(volatile s8 * const value, const s8 mask)
{
	return __sync_fetch_and_and(value, mask);
}

INTRIN_ACCESSOR s16 _InterlockedAnd16(volatile s16 * const value, const s16 mask)
{
	return __sync_fetch_and_and(value, mask);
}

INTRIN_ACCESSOR s32 _InterlockedAnd(volatile s32 * const value, const s32 mask)
{
	return __sync_fetch_and_and(value, mask);
}

#if defined(__x86_64__)
INTRIN_ACCESSOR s32 _InterlockedAnd64(volatile s64 * const value, const s64 mask)
{
	return __sync_fetch_and_and(value, mask);
}
#endif

INTRIN_ACCESSOR s8 _InterlockedOr8(volatile s8 * const value, const s8 mask)
{
	return __sync_fetch_and_or(value, mask);
}

INTRIN_ACCESSOR s16 _InterlockedOr16(volatile s16 * const value, const s16 mask)
{
	return __sync_fetch_and_or(value, mask);
}

INTRIN_ACCESSOR s32 _InterlockedOr(volatile s32 * const value, const s32 mask)
{
	return __sync_fetch_and_or(value, mask);
}

#if defined(__x86_64__)
INTRIN_ACCESSOR s32 _InterlockedOr64(volatile s64 * const value, const s64 mask)
{
	return __sync_fetch_and_or(value, mask);
}
#endif

INTRIN_ACCESSOR s8 _InterlockedXor8(volatile s8 * const value, const s8 mask)
{
	return __sync_fetch_and_xor(value, mask);
}

INTRIN_ACCESSOR s16 _InterlockedXor16(volatile s16 * const value, const s16 mask)
{
	return __sync_fetch_and_xor(value, mask);
}

INTRIN_ACCESSOR s32 _InterlockedXor(volatile s32 * const value, const s32 mask)
{
	return __sync_fetch_and_xor(value, mask);
}

#else

INTRIN_ACCESSOR s8 _InterlockedCompareExchange8(volatile s8 * const Destination, const s8 Exchange, const s8 Comperand)
{
	s8 retval = Comperand;
	__asm__("lock; cmpxchgb %b[Exchange], %[Destination]" : [retval] "+a" (retval) : [Destination] "m" (*Destination), [Exchange] "q" (Exchange) : "memory");
	return retval;
}

INTRIN_ACCESSOR s16 _InterlockedCompareExchange16(volatile s16 * const Destination, const s16 Exchange, const s16 Comperand)
{
	s16 retval = Comperand;
	__asm__("lock; cmpxchgw %w[Exchange], %[Destination]" : [retval] "+a" (retval) : [Destination] "m" (*Destination), [Exchange] "q" (Exchange): "memory");
	return retval;
}

INTRIN_ACCESSOR s32 _InterlockedCompareExchange(volatile s32 * const Destination, const s32 Exchange, const s32 Comperand)
{
	s32 retval = Comperand;
	__asm__("lock; cmpxchgl %k[Exchange], %[Destination]" : [retval] "+a" (retval) : [Destination] "m" (*Destination), [Exchange] "q" (Exchange): "memory");
	return retval;
}

INTRIN_ACCESSOR void * _InterlockedCompareExchangePointer(void * volatile * const Destination, void * const Exchange, void * const Comperand)
{
	void * retval = (void *)Comperand;
	__asm__("lock; cmpxchgl %k[Exchange], %[Destination]" : [retval] "=a" (retval) : "[retval]" (retval), [Destination] "m" (*Destination), [Exchange] "q" (Exchange) : "memory");
	return retval;
}

INTRIN_ACCESSOR s32 _InterlockedExchange(volatile s32 * const Target, const s32 Value)
{
	s32 retval = Value;
	__asm__("xchgl %[retval], %[Target]" : [retval] "+r" (retval) : [Target] "m" (*Target) : "memory");
	return retval;
}

INTRIN_ACCESSOR void * _InterlockedExchangePointer(void * volatile * const Target, void * const Value)
{
	void * retval = Value;
	__asm__("xchgl %[retval], %[Target]" : [retval] "+r" (retval) : [Target] "m" (*Target) : "memory");
	return retval;
}

INTRIN_ACCESSOR s32 _InterlockedExchangeAdd16(volatile s16 * const Addend, const s16 Value)
{
	s32 retval = Value;
	__asm__("lock; xaddw %[retval], %[Addend]" : [retval] "+r" (retval) : [Addend] "m" (*Addend) : "memory");
	return retval;
}

INTRIN_ACCESSOR s32 _InterlockedExchangeAdd(volatile s32 * const Addend, const s32 Value)
{
	s32 retval = Value;
	__asm__("lock; xaddl %[retval], %[Addend]" : [retval] "+r" (retval) : [Addend] "m" (*Addend) : "memory");
	return retval;
}

INTRIN_INLINE s8 _InterlockedAnd8(volatile s8 * const value, const s8 mask)
{
	s8 x;
	s8 y;

	y = *value;

	do
	{
		x = y;
		y = _InterlockedCompareExchange8(value, x & mask, x);
	}
	while(y != x);

	return y;
}

INTRIN_INLINE s16 _InterlockedAnd16(volatile s16 * const value, const s16 mask)
{
	s16 x;
	s16 y;

	y = *value;

	do
	{
		x = y;
		y = _InterlockedCompareExchange16(value, x & mask, x);
	}
	while(y != x);

	return y;
}

INTRIN_INLINE s32 _InterlockedAnd(volatile s32 * const value, const s32 mask)
{
	s32 x;
	s32 y;

	y = *value;

	do
	{
		x = y;
		y = _InterlockedCompareExchange(value, x & mask, x);
	}
	while(y != x);

	return y;
}

INTRIN_INLINE s8 _InterlockedOr8(volatile s8 * const value, const s8 mask)
{
	s8 x;
	s8 y;

	y = *value;

	do
	{
		x = y;
		y = _InterlockedCompareExchange8(value, x | mask, x);
	}
	while(y != x);

	return y;
}

INTRIN_INLINE s16 _InterlockedOr16(volatile s16 * const value, const s16 mask)
{
	s16 x;
	s16 y;

	y = *value;

	do
	{
		x = y;
		y = _InterlockedCompareExchange16(value, x | mask, x);
	}
	while(y != x);

	return y;
}

INTRIN_INLINE s32 _InterlockedOr(volatile s32 * const value, const s32 mask)
{
	s32 x;
	s32 y;

	y = *value;

	do
	{
		x = y;
		y = _InterlockedCompareExchange(value, x | mask, x);
	}
	while(y != x);

	return y;
}

INTRIN_INLINE s8 _InterlockedXor8(volatile s8 * const value, const s8 mask)
{
	s8 x;
	s8 y;

	y = *value;

	do
	{
		x = y;
		y = _InterlockedCompareExchange8(value, x ^ mask, x);
	}
	while(y != x);

	return y;
}

INTRIN_INLINE s16 _InterlockedXor16(volatile s16 * const value, const s16 mask)
{
	s16 x;
	s16 y;

	y = *value;

	do
	{
		x = y;
		y = _InterlockedCompareExchange16(value, x ^ mask, x);
	}
	while(y != x);

	return y;
}

INTRIN_INLINE s32 _InterlockedXor(volatile s32 * const value, const s32 mask)
{
	s32 x;
	s32 y;

	y = *value;

	do
	{
		x = y;
		y = _InterlockedCompareExchange(value, x ^ mask, x);
	}
	while(y != x);

	return y;
}

#endif

#if (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__) > 40100 && defined(__x86_64__)

INTRIN_ACCESSOR s64 _InterlockedCompareExchange64(volatile s64 * const Destination, const s64 Exchange, const s64 Comperand)
{
	return __sync_val_compare_and_swap(Destination, Comperand, Exchange);
}

#else

INTRIN_ACCESSOR s64 _InterlockedCompareExchange64(volatile s64 * const Destination, const s64 Exchange, const s64 Comperand)
{
	s64 retval = Comperand;

	__asm__
	(
		"lock; cmpxchg8b %[Destination]" :
		[retval] "+A" (retval) :
			[Destination] "m" (*Destination),
			"b" ((u32)((Exchange >>  0) & 0xFFFFFFFF)),
			"c" ((u32)((Exchange >> 32) & 0xFFFFFFFF)) :
		"memory"
	);

	return retval;
}

#endif

INTRIN_ACCESSOR s32 _InterlockedAddLargeStatistic(volatile s64 * const Addend, const s32 Value)
{
	__asm__
	(
		"lock; add %[Value], %[Lo32];"
		"jae LABEL%=;"
		"lock; adcl $0, %[Hi32];"
		"LABEL%=:;" :
		[Lo32] "+m" (*((volatile s32 *)(Addend) + 0)), [Hi32] "+m" (*((volatile s32 *)(Addend) + 1)) :
		[Value] "ir" (Value) :
		"memory"
	);

	return Value;
}

INTRIN_ACCESSOR s32 _InterlockedDecrement(volatile s32 * const lpAddend)
{
	return _InterlockedExchangeAdd(lpAddend, -1) - 1;
}

INTRIN_ACCESSOR s32 _InterlockedIncrement(volatile s32 * const lpAddend)
{
	return _InterlockedExchangeAdd(lpAddend, 1) + 1;
}

INTRIN_ACCESSOR s16 _InterlockedDecrement16(volatile s16 * const lpAddend)
{
	return _InterlockedExchangeAdd16(lpAddend, -1) - 1;
}

INTRIN_ACCESSOR s16 _InterlockedIncrement16(volatile s16 * const lpAddend)
{
	return _InterlockedExchangeAdd16(lpAddend, 1) + 1;
}

#if defined(__x86_64__)
INTRIN_ACCESSOR s64 _InterlockedDecrement64(volatile s64 * const lpAddend)
{
	return _InterlockedExchangeAdd64(lpAddend, -1) - 1;
}

INTRIN_ACCESSOR s64 _InterlockedIncrement64(volatile s64 * const lpAddend)
{
	return _InterlockedExchangeAdd64(lpAddend, 1) + 1;
}
#endif

INTRIN_ACCESSOR u8 _interlockedbittestandreset(volatile s32 * a, const s32 b)
{
	u8 retval;
	__asm__("lock; btrl %[b], %[a]; setb %b[retval]" : [retval] "=q" (retval), [a] "+m" (*a) : [b] "Ir" (b) : "memory");
	return retval;
}

#if defined(__x86_64__)
INTRIN_ACCESSOR u8 _interlockedbittestandreset64(volatile s64 * a, const s64 b)
{
	u8 retval;
	__asm__("lock; btrq %[b], %[a]; setb %b[retval]" : [retval] "=r" (retval), [a] "+m" (*a) : [b] "Ir" (b) : "memory");
	return retval;
}
#endif

INTRIN_ACCESSOR u8 _interlockedbittestandset(volatile s32 * a, const s32 b)
{
	u8 retval;
	__asm__("lock; btsl %[b], %[a]; setc %b[retval]" : [retval] "=q" (retval), [a] "+m" (*a) : [b] "Ir" (b) : "memory");
	return retval;
}

#if defined(__x86_64__)
INTRIN_ACCESSOR u8 _interlockedbittestandset64(volatile s64 * a, const s64 b)
{
	u8 retval;
	__asm__("lock; btsq %[b], %[a]; setc %b[retval]" : [retval] "=r" (retval), [a] "+m" (*a) : [b] "Ir" (b) : "memory");
	return retval;
}
#endif

/*** String operations ***/
/* NOTE: we don't set a memory clobber in the __stosX functions because Visual C++ doesn't */
INTRIN_ACCESSOR void __stosb(u8 * Dest, const u8 Data, size_t Count)
{
	__asm__ __volatile__
	(
		"rep; stosb" :
		[Dest] "=D" (Dest), [Count] "=c" (Count) :
		"[Dest]" (Dest), "a" (Data), "[Count]" (Count)
	);
}

INTRIN_ACCESSOR void __stosw(u16 * Dest, const u16 Data, size_t Count)
{
	__asm__ __volatile__
	(
		"rep; stosw" :
		[Dest] "=D" (Dest), [Count] "=c" (Count) :
		"[Dest]" (Dest), "a" (Data), "[Count]" (Count)
	);
}

INTRIN_ACCESSOR void __stosd(u32 * Dest, const u32 Data, size_t Count)
{
	__asm__ __volatile__
	(
		"rep; stosl" :
		[Dest] "=D" (Dest), [Count] "=c" (Count) :
		"[Dest]" (Dest), "a" (Data), "[Count]" (Count)
	);
}

#ifdef __x86_64__
INTRIN_ACCESSOR void __stosq(uint64_t * Dest, const uint64_t Data, size_t Count)
{
	__asm__ __volatile__
	(
		"rep; stosq" :
		[Dest] "=D" (Dest), [Count] "=c" (Count) :
		"[Dest]" (Dest), "a" (Data), "[Count]" (Count)
	);
}
#endif

INTRIN_ACCESSOR void __movsb(u8 * Destination, const u8 * Source, size_t Count)
{
	__asm__ __volatile__
	(
		"rep; movsb" :
		[Destination] "=D" (Destination), [Source] "=S" (Source), [Count] "=c" (Count) :
		"[Destination]" (Destination), "[Source]" (Source), "[Count]" (Count)
	);
}

INTRIN_ACCESSOR void __movsw(u16 * Destination, const u16 * Source, size_t Count)
{
	__asm__ __volatile__
	(
		"rep; movsw" :
		[Destination] "=D" (Destination), [Source] "=S" (Source), [Count] "=c" (Count) :
		"[Destination]" (Destination), "[Source]" (Source), "[Count]" (Count)
	);
}

INTRIN_ACCESSOR void __movsd(void* Destination, const void* Source, size_t Count)
{
	__asm__ __volatile__
	(
		"rep; movsd" :
		[Destination] "=D" (Destination), [Source] "=S" (Source), [Count] "=c" (Count) :
		"[Destination]" (Destination), "[Source]" (Source), "[Count]" (Count)
	);
}

#ifdef __x86_64__
INTRIN_ACCESSOR void __movsq(u64 * Destination, const u64 * Source, size_t Count)
{
	__asm__ __volatile__
	(
		"rep; movsq" :
		[Destination] "=D" (Destination), [Source] "=S" (Source), [Count] "=c" (Count) :
		"[Destination]" (Destination), "[Source]" (Source), "[Count]" (Count)
	);
}
#endif

#if defined(__x86_64__)
/*** GS segment addressing ***/

INTRIN_ACCESSOR void __writegsbyte(const u32 Offset, const u8 Data)
{
	__asm__ __volatile__("movb %b[Data], %%gs:%a[Offset]" : : [Offset] "ir" (Offset), [Data] "ir" (Data) : "memory");
}

INTRIN_ACCESSOR void __writegsword(const u32 Offset, const u16 Data)
{
	__asm__ __volatile__("movw %w[Data], %%gs:%a[Offset]" : : [Offset] "ir" (Offset), [Data] "ir" (Data) : "memory");
}

INTRIN_ACCESSOR void __writegsdword(const u32 Offset, const u32 Data)
{
	__asm__ __volatile__("movl %k[Data], %%gs:%a[Offset]" : : [Offset] "ir" (Offset), [Data] "ir" (Data) : "memory");
}

INTRIN_ACCESSOR void __writegsqword(const u32 Offset, const uint64_t Data)
{
	__asm__ __volatile__("movq %q[Data], %%gs:%a[Offset]" : : [Offset] "ir" (Offset), [Data] "ir" (Data) : "memory");
}

INTRIN_ACCESSOR u8 __readgsbyte(const u32 Offset)
{
	u8 value;
	__asm__ __volatile__("movb %%gs:%a[Offset], %b[value]" : [value] "=r" (value) : [Offset] "ir" (Offset));
	return value;
}

INTRIN_ACCESSOR u16 __readgsword(const u32 Offset)
{
	u16 value;
	__asm__ __volatile__("movw %%gs:%a[Offset], %w[value]" : [value] "=r" (value) : [Offset] "ir" (Offset));
	return value;
}

INTRIN_ACCESSOR u32 __readgsdword(const u32 Offset)
{
	u32 value;
	__asm__ __volatile__("movl %%gs:%a[Offset], %k[value]" : [value] "=r" (value) : [Offset] "ir" (Offset));
	return value;
}

INTRIN_ACCESSOR uint64_t __readgsqword(const u32 Offset)
{
	uint64_t value;
	__asm__ __volatile__("movq %%gs:%a[Offset], %q[value]" : [value] "=r" (value) : [Offset] "ir" (Offset));
	return value;
}

INTRIN_ACCESSOR void __incgsbyte(const u32 Offset)
{
	__asm__ __volatile__("incb %%gs:%a[Offset]" : : [Offset] "ir" (Offset) : "memory");
}

INTRIN_ACCESSOR void __incgsword(const u32 Offset)
{
	__asm__ __volatile__("incw %%gs:%a[Offset]" : : [Offset] "ir" (Offset) : "memory");
}

INTRIN_ACCESSOR void __incgsdword(const u32 Offset)
{
	__asm__ __volatile__("incl %%gs:%a[Offset]" : : [Offset] "ir" (Offset) : "memory");
}

INTRIN_ACCESSOR void __addgsbyte(const u32 Offset, const u8 Data)
{
	__asm__ __volatile__("addb %b[Data], %%gs:%a[Offset]" : : [Offset] "ir" (Offset), [Data] "ir" (Data) : "memory");
}

INTRIN_ACCESSOR void __addgsword(const u32 Offset, const u16 Data)
{
	__asm__ __volatile__("addw %w[Data], %%gs:%a[Offset]" : : [Offset] "ir" (Offset), [Data] "ir" (Data) : "memory");
}

INTRIN_ACCESSOR void __addgsdword(const u32 Offset, const unsigned int Data)
{
	__asm__ __volatile__("addl %k[Data], %%gs:%a[Offset]" : : [Offset] "ir" (Offset), [Data] "ir" (Data) : "memory");
}

#if 0
// generates an Error: incorrect register `%edx' used with `q' suffix
// Not sure how to fix it properly, and doubt direct GS segment access will ever be needed anyway,
// so just stripping it out.  --jstine
INTRIN_ACCESSOR void __addgsqword(const u32 Offset, const u64 Data)
{
	__asm__ __volatile__("addq %k[Data], %%gs:%a[Offset]" : : [Offset] "ir" (Offset), [Data] "ir" (Data) : "memory");
}
#endif

#else
/*** FS segment addressing ***/
INTRIN_ACCESSOR void __writefsbyte(const u32 Offset, const u8 Data)
{
	__asm__ __volatile__("movb %b[Data], %%fs:%a[Offset]" : : [Offset] "ir" (Offset), [Data] "iq" (Data) : "memory");
}

INTRIN_ACCESSOR void __writefsword(const u32 Offset, const u16 Data)
{
	__asm__ __volatile__("movw %w[Data], %%fs:%a[Offset]" : : [Offset] "ir" (Offset), [Data] "ir" (Data) : "memory");
}

INTRIN_ACCESSOR void __writefsdword(const u32 Offset, const u32 Data)
{
	__asm__ __volatile__("movl %k[Data], %%fs:%a[Offset]" : : [Offset] "ir" (Offset), [Data] "ir" (Data) : "memory");
}

INTRIN_ACCESSOR u8 __readfsbyte(const u32 Offset)
{
	u8 value;
	__asm__ __volatile__("movb %%fs:%a[Offset], %b[value]" : [value] "=q" (value) : [Offset] "ir" (Offset));
	return value;
}

INTRIN_ACCESSOR u16 __readfsword(const u32 Offset)
{
	u16 value;
	__asm__ __volatile__("movw %%fs:%a[Offset], %w[value]" : [value] "=r" (value) : [Offset] "ir" (Offset));
	return value;
}

INTRIN_ACCESSOR u32 __readfsdword(const u32 Offset)
{
	u32 value;
	__asm__ __volatile__("movl %%fs:%a[Offset], %k[value]" : [value] "=r" (value) : [Offset] "ir" (Offset));
	return value;
}

INTRIN_ACCESSOR void __incfsbyte(const u32 Offset)
{
	__asm__ __volatile__("incb %%fs:%a[Offset]" : : [Offset] "ir" (Offset) : "memory");
}

INTRIN_ACCESSOR void __incfsword(const u32 Offset)
{
	__asm__ __volatile__("incw %%fs:%a[Offset]" : : [Offset] "ir" (Offset) : "memory");
}

INTRIN_ACCESSOR void __incfsdword(const u32 Offset)
{
	__asm__ __volatile__("incl %%fs:%a[Offset]" : : [Offset] "ir" (Offset) : "memory");
}

/* NOTE: the bizarre implementation of __addfsxxx mimics the broken Visual C++ behavior */
INTRIN_ACCESSOR void __addfsbyte(const u32 Offset, const u8 Data)
{
	if(!__builtin_constant_p(Offset))
		__asm__ __volatile__("addb %b[Offset], %%fs:%a[Offset]" : : [Offset] "r" (Offset) : "memory");
	else
		__asm__ __volatile__("addb %b[Data], %%fs:%a[Offset]" : : [Offset] "ir" (Offset), [Data] "iq" (Data) : "memory");
}

INTRIN_ACCESSOR void __addfsword(const u32 Offset, const u16 Data)
{
	if(!__builtin_constant_p(Offset))
		__asm__ __volatile__("addw %w[Offset], %%fs:%a[Offset]" : : [Offset] "r" (Offset) : "memory");
	else
		__asm__ __volatile__("addw %w[Data], %%fs:%a[Offset]" : : [Offset] "ir" (Offset), [Data] "iq" (Data) : "memory");
}

INTRIN_ACCESSOR void __addfsdword(const u32 Offset, const unsigned int Data)
{
	if(!__builtin_constant_p(Offset))
		__asm__ __volatile__("addl %k[Offset], %%fs:%a[Offset]" : : [Offset] "r" (Offset) : "memory");
	else
		__asm__ __volatile__("addl %k[Data], %%fs:%a[Offset]" : : [Offset] "ir" (Offset), [Data] "iq" (Data) : "memory");
}
#endif


/*** Bit manipulation ***/
INTRIN_ACCESSOR u8 _BitScanForward(u32 * const Index, const u32 Mask)
{
	__asm__("bsfl %[Mask], %[Index]" : [Index] "=r" (*Index) : [Mask] "mr" (Mask));
	return Mask ? 1 : 0;
}

INTRIN_ACCESSOR u8 _BitScanReverse(u32 * const Index, const u32 Mask)
{
	__asm__("bsrl %[Mask], %[Index]" : [Index] "=r" (*Index) : [Mask] "mr" (Mask));
	return Mask ? 1 : 0;
}

#ifdef __x86_64__
INTRIN_ACCESSOR u8 _BitScanForward64(u64 * const Index, const u64 Mask)
{
	__asm__("bsfq %[Mask], %[Index]" : [Index] "=r" (*Index) : [Mask] "mr" (Mask));
	return Mask ? 1 : 0;
}

INTRIN_ACCESSOR u8 _BitScanReverse64(u64 * const Index, const u64 Mask)
{
	__asm__("bsrq %[Mask], %[Index]" : [Index] "=r" (*Index) : [Mask] "mr" (Mask));
	return Mask ? 1 : 0;
}
#endif


/* NOTE: again, the bizarre implementation follows Visual C++ */
INTRIN_ACCESSOR u8 _bittest(const s32 * const a, const s32 b)
{
	u8 retval;

	if(__builtin_constant_p(b))
		__asm__("bt %[b], %[a]; setb %b[retval]" : [retval] "=q" (retval) : [a] "mr" (*(a + (b / 32))), [b] "Ir" (b % 32));
	else
		__asm__("bt %[b], %[a]; setb %b[retval]" : [retval] "=q" (retval) : [a] "mr" (*a), [b] "r" (b));

	return retval;
}

#ifdef __x86_64__
INTRIN_ACCESSOR u8 _bittest64(const int64_t * const a, const int64_t b)
{
	u8 retval;

	if(__builtin_constant_p(b))
		__asm__("bt %[b], %[a]; setb %b[retval]" : [retval] "=q" (retval) : [a] "mr" (*(a + (b / 64))), [b] "Ir" (b % 64));
	else
		__asm__("bt %[b], %[a]; setb %b[retval]" : [retval] "=q" (retval) : [a] "mr" (*a), [b] "r" (b));

	return retval;
}
#endif

INTRIN_ACCESSOR u8 _bittestandcomplement(s32 * const a, const s32 b)
{
	u8 retval;

	if(__builtin_constant_p(b))
		__asm__("btc %[b], %[a]; setb %b[retval]" : [a] "+mr" (*(a + (b / 32))), [retval] "=q" (retval) : [b] "Ir" (b % 32));
	else
		__asm__("btc %[b], %[a]; setb %b[retval]" : [a] "+mr" (*a), [retval] "=q" (retval) : [b] "r" (b));

	return retval;
}

#ifdef __x86_64__
INTRIN_ACCESSOR u8 _bittestandcompliment64(int64_t * const a, const int64_t b)
{
	u8 retval;

	if(__builtin_constant_p(b))
		__asm__("btc %[b], %[a]; setb %b[retval]" : [a] "+mr" (*(a + (b / 64))), [retval] "=q" (retval) : [b] "Ir" (b % 64));
	else
		__asm__("btc %[b], %[a]; setb %b[retval]" : [a] "+mr" (*a), [retval] "=q" (retval) : [b] "r" (b));

	return retval;
}
#endif

INTRIN_ACCESSOR u8 _bittestandreset(s32 * const a, const s32 b)
{
	u8 retval;

	if(__builtin_constant_p(b))
		__asm__("btr %[b], %[a]; setb %b[retval]" : [a] "+mr" (*(a + (b / 32))), [retval] "=q" (retval) : [b] "Ir" (b % 32));
	else
		__asm__("btr %[b], %[a]; setb %b[retval]" : [a] "+mr" (*a), [retval] "=q" (retval) : [b] "r" (b));

	return retval;
}

#ifdef __x86_64__
INTRIN_ACCESSOR u8 _bittestandreset64(int64_t * const a, const int64_t b)
{
	u8 retval;

	if(__builtin_constant_p(b))
		__asm__("btr %[b], %[a]; setb %b[retval]" : [a] "+mr" (*(a + (b / 64))), [retval] "=q" (retval) : [b] "Ir" (b % 64));
	else
		__asm__("btr %[b], %[a]; setb %b[retval]" : [a] "+mr" (*a), [retval] "=q" (retval) : [b] "r" (b));

	return retval;
}
#endif

INTRIN_ACCESSOR u8 _bittestandset(s32 * const a, const s32 b)
{
	u8 retval;

	if(__builtin_constant_p(b))
		__asm__("bts %[b], %[a]; setb %b[retval]" : [a] "+mr" (*(a + (b / 32))), [retval] "=q" (retval) : [b] "Ir" (b % 32));
	else
		__asm__("bts %[b], %[a]; setb %b[retval]" : [a] "+mr" (*a), [retval] "=q" (retval) : [b] "r" (b));

	return retval;
}

#ifdef __x86_64__
INTRIN_ACCESSOR u8 _bittestandset64(int64_t * const a, const int64_t b)
{
	u8 retval;

	if(__builtin_constant_p(b))
		__asm__("bts %[b], %[a]; setb %b[retval]" : [a] "+mr" (*(a + (b / 64))), [retval] "=q" (retval) : [b] "Ir" (b % 64));
	else
		__asm__("bts %[b], %[a]; setb %b[retval]" : [a] "+mr" (*a), [retval] "=q" (retval) : [b] "r" (b));

	return retval;
}
#endif

INTRIN_ACCESSOR u8 _rotl8(u8 value, u8 shift)
{
	u8 retval;
	__asm__("rolb %b[shift], %b[retval]" : [retval] "=rm" (retval) : "[retval]" (value), [shift] "Nc" (shift));
	return retval;
}

INTRIN_ACCESSOR u16 _rotl16(u16 value, u8 shift)
{
	u16 retval;
	__asm__("rolw %b[shift], %w[retval]" : [retval] "=rm" (retval) : "[retval]" (value), [shift] "Nc" (shift));
	return retval;
}

//  * rotl/rotr apparently provided by GCC's intrin stuff now, as of v4.2 or so.
//    (but it does not provide 8 or 32 bit versions!)
//  * rotl/rotr are NOT provided by CLANG v3.0

#if defined(__clang__)
INTRIN_ACCESSOR u32 _rotl(u32 value, int shift)
{
	u32 retval;
	__asm__("roll %b[shift], %k[retval]" : [retval] "=rm" (retval) : "[retval]" (value), [shift] "Nc" (shift));
	return retval;
}

INTRIN_ACCESSOR u32 _rotr(u32 value, int shift)
{
	u32 retval;
	__asm__("rorl %b[shift], %k[retval]" : [retval] "=rm" (retval) : "[retval]" (value), [shift] "Nc" (shift));
	return retval;
}
#endif

INTRIN_ACCESSOR u8 _rotr8(u8 value, u8 shift)
{
	u8 retval;
	__asm__("rorb %b[shift], %b[retval]" : [retval] "=rm" (retval) : "[retval]" (value), [shift] "Nc" (shift));
	return retval;
}

INTRIN_ACCESSOR u16 _rotr16(u16 value, u8 shift)
{
	u16 retval;
	__asm__("rorw %b[shift], %w[retval]" : [retval] "=rm" (retval) : "[retval]" (value), [shift] "Nc" (shift));
	return retval;
}

#if defined(__x86_64__)
INTRIN_ACCESSOR u64 _rotl64(u64 value, int shift)
{
	u64 retval;
	__asm__("rolq %b[shift], %q[retval]" : [retval] "=rm" (retval) : "[retval]" (value), [shift] "Nc" (shift));
	return retval;
}

INTRIN_ACCESSOR u64 _rotr64(u64 value, int shift)
{
	u64 retval;
	__asm__("rorq %b[shift], %q[retval]" : [retval] "=rm" (retval) : "[retval]" (value), [shift] "Nc" (shift));
	return retval;
}
#endif

/*
	NOTE: in __ll_lshift, __ll_rshift and __ull_rshift we use the "A"
	constraint (edx:eax) for the Mask argument, because it's the only way GCC
	can pass 64-bit operands around - passing the two 32 bit parts separately
	just confuses it. Also we declare Bit as an int and then truncate it to
	match Visual C++ behavior
*/
INTRIN_ACCESSOR u64 __ll_lshift(const u64 Mask, const int Bit)
{
	u64 retval = Mask;

	__asm__
	(
		"shldl %b[Bit], %%eax, %%edx; sall %b[Bit], %%eax" :
		"+A" (retval) :
		[Bit] "Nc" ((u8)((u32)Bit) & 0xFF)
	);

	return retval;
}

INTRIN_ACCESSOR s64 __ll_rshift(const s64 Mask, const int Bit)
{
	u64 retval = Mask;

	__asm__
	(
		"shldl %b[Bit], %%eax, %%edx; sarl %b[Bit], %%eax" :
		"+A" (retval) :
		[Bit] "Nc" ((u8)((u32)Bit) & 0xFF)
	);

	return retval;
}

INTRIN_ACCESSOR u64 __ull_rshift(const u64 Mask, int Bit)
{
	u64 retval = Mask;

	__asm__
	(
		"shrdl %b[Bit], %%eax, %%edx; shrl %b[Bit], %%eax" :
		"+A" (retval) :
		[Bit] "Nc" ((u8)((u32)Bit) & 0xFF)
	);

	return retval;
}

INTRIN_ACCESSOR u16 _byteswap_ushort(u16 value)
{
	u16 retval;
	__asm__("rorw $8, %w[retval]" : [retval] "=rm" (retval) : "[retval]" (value));
	return retval;
}

INTRIN_ACCESSOR u32 _byteswap_ulong(u32 value)
{
	u32 retval;
	__asm__("bswapl %[retval]" : [retval] "=r" (retval) : "[retval]" (value));
	return retval;
}

#ifdef __x86_64__
INTRIN_ACCESSOR u64 _byteswap_uint64(u64 value)
{
	u64 retval;
	__asm__("bswapq %[retval]" : [retval] "=r" (retval) : "[retval]" (value));
	return retval;
}
#else
INTRIN_ACCESSOR u64 _byteswap_uint64(u64 value)
{
	union {
		s64 int64part;
		struct {
			u32 lowpart;
			u32 hipart;
		};
	} retval;
	retval.int64part = value;
	__asm__("bswapl %[lowpart]\n"
	        "bswapl %[hipart]\n"
	        : [lowpart] "=r" (retval.hipart), [hipart] "=r" (retval.lowpart)  : "[lowpart]" (retval.lowpart), "[hipart]" (retval.hipart) );
	return retval.int64part;
}
#endif

/*** 64-bit math ***/
INTRIN_ACCESSOR s64 __emul(const int a, const int b)
{
	s64 retval;
	__asm__("imull %[b]" : "=A" (retval) : [a] "a" (a), [b] "rm" (b));
	return retval;
}

INTRIN_ACCESSOR u64 __emulu(const unsigned int a, const unsigned int b)
{
	u64 retval;
	__asm__("mull %[b]" : "=A" (retval) : [a] "a" (a), [b] "rm" (b));
	return retval;
}

#ifdef __x86_64__

INTRIN_ACCESSOR s64 __mulh(s64 a, s64 b)
{
	u64 retval;
	__asm__("imulq %[b]" : "=d" (retval) : [a] "a" (a), [b] "rm" (b));
	return retval;
}

INTRIN_ACCESSOR u64 __umulh(u64 a, u64 b)
{
	s64 retval;
	__asm__("mulq %[b]" : "=d" (retval) : [a] "a" (a), [b] "rm" (b));
	return retval;
}

#endif

/*** Port I/O ***/
INTRIN_ACCESSOR u8 __inbyte(const u16 Port)
{
	u8 byte;
	__asm__ __volatile__("inb %w[Port], %b[byte]" : [byte] "=a" (byte) : [Port] "Nd" (Port));
	return byte;
}

INTRIN_ACCESSOR u16 __inword(const u16 Port)
{
	u16 word;
	__asm__ __volatile__("inw %w[Port], %w[word]" : [word] "=a" (word) : [Port] "Nd" (Port));
	return word;
}

INTRIN_ACCESSOR u32 __indword(const u16 Port)
{
	u32 dword;
	__asm__ __volatile__("inl %w[Port], %k[dword]" : [dword] "=a" (dword) : [Port] "Nd" (Port));
	return dword;
}

INTRIN_ACCESSOR void __inbytestring(u16 Port, u8 * Buffer, u32 Count)
{
	__asm__ __volatile__
	(
		"rep; insb" :
		[Buffer] "=D" (Buffer), [Count] "=c" (Count) :
		"d" (Port), "[Buffer]" (Buffer), "[Count]" (Count) :
		"memory"
	);
}

INTRIN_ACCESSOR void __inwordstring(u16 Port, u16 * Buffer, u32 Count)
{
	__asm__ __volatile__
	(
		"rep; insw" :
		[Buffer] "=D" (Buffer), [Count] "=c" (Count) :
		"d" (Port), "[Buffer]" (Buffer), "[Count]" (Count) :
		"memory"
	);
}

INTRIN_ACCESSOR void __indwordstring(u16 Port, u32 * Buffer, u32 Count)
{
	__asm__ __volatile__
	(
		"rep; insl" :
		[Buffer] "=D" (Buffer), [Count] "=c" (Count) :
		"d" (Port), "[Buffer]" (Buffer), "[Count]" (Count) :
		"memory"
	);
}

INTRIN_ACCESSOR void __outbyte(u16 const Port, const u8 Data)
{
	__asm__ __volatile__("outb %b[Data], %w[Port]" : : [Port] "Nd" (Port), [Data] "a" (Data));
}

INTRIN_ACCESSOR void __outword(u16 const Port, const u16 Data)
{
	__asm__ __volatile__("outw %w[Data], %w[Port]" : : [Port] "Nd" (Port), [Data] "a" (Data));
}

INTRIN_ACCESSOR void __outdword(u16 const Port, const u32 Data)
{
	__asm__ __volatile__("outl %k[Data], %w[Port]" : : [Port] "Nd" (Port), [Data] "a" (Data));
}

INTRIN_ACCESSOR void __outbytestring(u16 const Port, const u8 * const Buffer, const u32 Count)
{
	__asm__ __volatile__("rep; outsb" : : [Port] "d" (Port), [Buffer] "S" (Buffer), "c" (Count));
}

INTRIN_ACCESSOR void __outwordstring(u16 const Port, const u16 * const Buffer, const u32 Count)
{
	__asm__ __volatile__("rep; outsw" : : [Port] "d" (Port), [Buffer] "S" (Buffer), "c" (Count));
}

INTRIN_ACCESSOR void __outdwordstring(u16 const Port, const u32 * const Buffer, const u32 Count)
{
	__asm__ __volatile__("rep; outsl" : : [Port] "d" (Port), [Buffer] "S" (Buffer), "c" (Count));
}


/*** System information ***/
INTRIN_ACCESSOR void __cpuid(int CPUInfo[], const int InfoType)
{
	__asm__ __volatile__("cpuid" : "=a" (CPUInfo[0]), "=b" (CPUInfo[1]), "=c" (CPUInfo[2]), "=d" (CPUInfo[3]) : "a" (InfoType));
}

#if !defined(HAS_RDTSC)
#	if defined(SCE_ORBIS_SDK_VERSION) && ((SCE_ORBIS_SDK_VERSION >> 16) >= 0x0200)
#		define HAS_RDTSC		1
#	elif (__clang_major__ > 3) || ((__clang_major__ == 3) && (__clang_minor__ >= 6))
#		define HAS_RDTSC		1
#	endif
#endif

#if !defined(HAS_READEFLAGS)
	// SCE clang has its own __readeflags intrinsic from SDK2500(ia32intrin.h).
#	if defined(SCE_ORBIS_SDK_VERSION) && ((SCE_ORBIS_SDK_VERSION >> 16) >= 0x0250)
#		define HAS_READEFLAGS		1
#	elif (__clang_major__ > 3) || ((__clang_major__ == 3) && (__clang_minor__ >= 6))
#		define HAS_READEFLAGS		1
#	endif
#endif

#if !defined(HAS_RDTSC)
#	define HAS_RDTSC				0
#endif


#if !defined(HAS_READEFLAGS)
#	define HAS_READEFLAGS			0
#endif

#if !HAS_RDTSC
// typically  included in ia32intrin.h  (via x86intrin.h)
//  (but only in SDK 2.00+ on Orbis)
INTRIN_ACCESSOR u64 __rdtsc(void)
{
#ifdef __x86_64__
	u64 low, high;
	__asm__ __volatile__("rdtsc" : "=a"(low), "=d"(high));
	return low | (high << 32);
#else
	u64 retval;
	__asm__ __volatile__("rdtsc" : "=A"(retval));
	return retval;
#endif
}
#endif

INTRIN_ACCESSOR void __writeeflags(uintptr_t Value)
{
	__asm__ __volatile__("push %0\n popf" : : "rim"(Value));
}

#if !HAS_READEFLAGS
INTRIN_ACCESSOR uintptr_t __readeflags(void)
{
	uintptr_t retval;
	__asm__ __volatile__("pushf\n pop %0" : "=rm"(retval));
	return retval;
}
#endif

/*** Interrupts ***/
#if !defined(SCE_ORBIS_SDK_VERSION) && !(defined(__clang__) && defined(__GNUC__) && (__clang_major__ == 3) && (__clang_minor__ == 3))
// SCE clang has its own __debugbreak intrinsic
// clang on linux v3.3 also does.  But 3.2 and 3.4 do not.  (ugh)  --jstine
INTRIN_ACCESSOR void __debugbreak(void)
{
	__asm__("int $3");
}
#endif

INTRIN_ACCESSOR void __int2c(void)
{
	__asm__("int $0x2c");
}

INTRIN_ACCESSOR void _disable(void)
{
	__asm__("cli");
}

INTRIN_ACCESSOR void _enable(void)
{
	__asm__("sti");
}

INTRIN_ACCESSOR void __halt(void)
{
	__asm__("hlt\n\t");
}

#if 0
INTRIN_ACCESSOR void _mm_pause(void)
{
	__asm__ __volatile__("pause");
}
#endif

#ifdef __cplusplus
}
#endif

#endif // GCC

#endif // KJK_INTRIN_X86_H_

