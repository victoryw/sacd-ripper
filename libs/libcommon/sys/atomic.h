/*
 * PowerPC atomic operations
 *
 * Copied from the linux 2.6.x kernel sources:
 *  - removed all kernel dependencies
 *  - removed PPC_ACQUIRE_BARRIER, PPC_RELEASE_BARRIER macros
 *  - removed PPC405_ERR77 macro
 *
 */

#ifndef _ASM_POWERPC_ATOMIC_H_
#define _ASM_POWERPC_ATOMIC_H_

#ifndef __lv2ppu__
#error you need the psl1ght/lv2 ppu compatible compiler!
#endif

#include <stdint.h>

typedef struct { volatile uint32_t counter; } atomic_t; 
typedef struct { uint64_t counter; } atomic64_t;

static inline uint32_t atomic_read(const atomic_t *v)
{
        uint32_t t;

        __asm__ volatile("lwz%U1%X1 %0,%1" : "=r"(t) : "m"(v->counter));

        return t;
}

static inline void atomic_set(atomic_t *v, int i)
{
        __asm__ volatile("stw%U0%X0 %1,%0" : "=m"(v->counter) : "r"(i));
}

static inline void atomic_add(uint32_t a, atomic_t *v)
{
        uint32_t t;

        __asm__ volatile(
"1:     lwarx   %0,0,%3         # atomic_add\n\
        add     %0,%2,%0\n"
"       stwcx.  %0,0,%3 \n\
        bne-    1b"
        : "=&r" (t), "+m" (v->counter)
        : "r" (a), "r" (&v->counter)
        : "cc");
}

static inline uint32_t atomic_add_return(uint32_t a, atomic_t *v)
{
        uint32_t t;

        __asm__ volatile(
"1:     lwarx   %0,0,%2         # atomic_add_return\n\
        add     %0,%1,%0\n"
"       stwcx.  %0,0,%2 \n\
        bne-    1b"
        : "=&r" (t)
        : "r" (a), "r" (&v->counter)
        : "cc", "memory");

        return t;
}

#define atomic_add_negative(a, v)       (atomic_add_return((a), (v)) < 0)

static inline void atomic_sub(uint32_t a, atomic_t *v)
{
        uint32_t t;

        __asm__ volatile(
"1:     lwarx   %0,0,%3         # atomic_sub\n\
        subf    %0,%2,%0\n"
"       stwcx.  %0,0,%3 \n\
        bne-    1b"
        : "=&r" (t), "+m" (v->counter)
        : "r" (a), "r" (&v->counter)
        : "cc");
}

static inline uint32_t atomic_sub_return(uint32_t a, atomic_t *v)
{
        uint32_t t;

        __asm__ volatile(
"1:     lwarx   %0,0,%2         # atomic_sub_return\n\
        subf    %0,%1,%0\n"
"       stwcx.  %0,0,%2 \n\
        bne-    1b"
        : "=&r" (t)
        : "r" (a), "r" (&v->counter)
        : "cc", "memory");

        return t;
}

static inline void atomic_inc(atomic_t *v)
{
        uint32_t t;

        __asm__ volatile(
"1:     lwarx   %0,0,%2         # atomic_inc\n\
        addic   %0,%0,1\n"
"       stwcx.  %0,0,%2 \n\
        bne-    1b"
        : "=&r" (t), "+m" (v->counter)
        : "r" (&v->counter)
        : "cc", "xer");
}

static inline uint32_t atomic_inc_return(atomic_t *v)
{
        uint32_t t;

        __asm__ volatile(
"1:     lwarx   %0,0,%1         # atomic_inc_return\n\
        addic   %0,%0,1\n"
"       stwcx.  %0,0,%1 \n\
        bne-    1b"
        : "=&r" (t)
        : "r" (&v->counter)
        : "cc", "xer", "memory");

        return t;
}

/*
 * atomic_inc_and_test - increment and test
 * @v: pointer of type atomic_t
 *
 * Atomically increments @v by 1
 * and returns true if the result is zero, or false for all
 * other cases.
 */
#define atomic_inc_and_test(v) (atomic_inc_return(v) == 0)

static inline void atomic_dec(atomic_t *v)
{
        uint32_t t;

        __asm__ volatile(
"1:     lwarx   %0,0,%2         # atomic_dec\n\
        addic   %0,%0,-1\n"
"       stwcx.  %0,0,%2\n\
        bne-    1b"
        : "=&r" (t), "+m" (v->counter)
        : "r" (&v->counter)
        : "cc", "xer");
}

static inline uint32_t atomic_dec_return(atomic_t *v)
{
        uint32_t t;

        __asm__ volatile(
"1:     lwarx   %0,0,%1         # atomic_dec_return\n\
        addic   %0,%0,-1\n"
"       stwcx.  %0,0,%1\n\
        bne-    1b"
        : "=&r" (t)
        : "r" (&v->counter)
        : "cc", "xer", "memory");

        return t;
}

/*
 * Atomic exchange
 *
 * Changes the memory location '*ptr' to be val and returns
 * the previous value stored there.
 */
static inline uint32_t __xchg_u32(volatile void *p, uint32_t val)
{
	uint32_t prev;

	__asm__ volatile(
"1:	lwarx	%0,0,%2 \n"
"	stwcx.	%3,0,%2 \n\
	bne-	1b"
	: "=&r" (prev), "+m" (*(volatile unsigned int *)p)
	: "r" (p), "r" (val)
	: "cc", "memory");

	return prev;
}

static inline uint32_t __xchg_u64(volatile void *p, uint32_t val)
{
	uint32_t prev;

	__asm__ volatile(
"1:	ldarx	%0,0,%2 \n"
"	stdcx.	%3,0,%2 \n\
	bne-	1b"
	: "=&r" (prev), "+m" (*(volatile uint32_t *)p)
	: "r" (p), "r" (val)
	: "cc", "memory");

	return prev;
}

/*
 * This function doesn't exist, so you'll get a linker error
 * if something tries to do an invalid xchg().
 */
extern void __xchg_called_with_bad_pointer(void);

static inline uint32_t __xchg(volatile void *ptr, uint32_t x, unsigned int size)
{
	switch (size) {
	case 4:
		return __xchg_u32(ptr, x);
	case 8:
		return __xchg_u64(ptr, x);
	}
	__xchg_called_with_bad_pointer();
	return x;
}

#define xchg(ptr,x)							     \
  ({									         \
     __typeof__(*(ptr)) _x_ = (x);				 \
     (__typeof__(*(ptr))) __xchg((ptr), (uint32_t)_x_, sizeof(*(ptr))); \
  })

/*
 * Compare and exchange - if *p == old, set it to new,
 * and return the old value of *p.
 */
static inline uint64_t
__cmpxchg_u32(volatile unsigned int *p, uint64_t old, uint64_t new)
{
	unsigned int prev;

	__asm__ volatile (
"1:	lwarx	%0,0,%2		# __cmpxchg_u32\n\
	cmpw	0,%0,%3\n\
	bne-	2f\n"
"	stwcx.	%4,0,%2\n\
	bne-	1b"
	"\n\
2:"
	: "=&r" (prev), "+m" (*p)
	: "r" (p), "r" (old), "r" (new)
	: "cc", "memory");

	return prev;
}

static inline uint64_t
__cmpxchg_u64(volatile uint64_t *p, uint64_t old, uint64_t new)
{
	uint64_t prev;

	__asm__ volatile (
"1:	ldarx	%0,0,%2		# __cmpxchg_u64\n\
	cmpd	0,%0,%3\n\
	bne-	2f\n\
	stdcx.	%4,0,%2\n\
	bne-	1b"
	"\n\
2:"
	: "=&r" (prev), "+m" (*p)
	: "r" (p), "r" (old), "r" (new)
	: "cc", "memory");

	return prev;
}

/* This function doesn't exist, so you'll get a linker error
   if something tries to do an invalid cmpxchg().  */
extern void __cmpxchg_called_with_bad_pointer(void);

static inline uint64_t
__cmpxchg(volatile void *ptr, uint64_t old, uint64_t new,
	  unsigned int size)
{
	switch (size) {
	case 4:
		return __cmpxchg_u32(ptr, old, new);
	case 8:
		return __cmpxchg_u64(ptr, old, new);
	}
	__cmpxchg_called_with_bad_pointer();
	return old;
}

#define cmpxchg(ptr, o, n)						 \
  ({									 \
     __typeof__(*(ptr)) _o_ = (o);					 \
     __typeof__(*(ptr)) _n_ = (n);					 \
     (__typeof__(*(ptr))) __cmpxchg((ptr), (uint64_t)_o_,		 \
				    (uint64_t)_n_, sizeof(*(ptr))); \
  })

#define atomic_cmpxchg(v, o, n) (cmpxchg(&((v)->counter), (o), (n)))
#define atomic_xchg(v, new) (xchg(&((v)->counter), new))

/**
 * atomic_add_unless - add unless the number is a given value
 * @v: pointer of type atomic_t
 * @a: the amount to add to v...
 * @u: ...unless v is equal to u.
 *
 * Atomically adds @a to @v, so long as it was not @u.
 * Returns non-zero if @v was not @u, and zero otherwise.
 */
static inline uint32_t atomic_add_unless(atomic_t *v, uint32_t a, int u)
{
        uint32_t t;

        __asm__ volatile (
"1:     lwarx   %0,0,%1         # atomic_add_unless\n\
        cmpw    0,%0,%3 \n\
        beq-    2f \n\
        add     %0,%2,%0 \n"
"       stwcx.  %0,0,%1 \n\
        bne-    1b \n"
"       subf    %0,%2,%0 \n\
2:"
        : "=&r" (t)
        : "r" (&v->counter), "r" (a), "r" (u)
        : "cc", "memory");

        return t != u;
}

#define atomic_inc_not_zero(v) atomic_add_unless((v), 1, 0)

#define atomic_sub_and_test(a, v)       (atomic_sub_return((a), (v)) == 0)
#define atomic_dec_and_test(v)          (atomic_dec_return((v)) == 0)

/*
 * Atomically test *v and decrement if it is greater than 0.
 * The function returns the old value of *v minus 1, even if
 * the atomic variable, v, was not decremented.
 */
static inline uint32_t atomic_dec_if_positive(atomic_t *v)
{
        uint32_t t;

        __asm__ volatile(
"1:     lwarx   %0,0,%1         # atomic_dec_if_positive\n\
        cmpwi   %0,1\n\
        addi    %0,%0,-1\n\
        blt-    2f\n"
"       stwcx.  %0,0,%1\n\
        bne-    1b"
        "\n\
2:"     : "=&b" (t)
        : "r" (&v->counter)
        : "cc", "memory");

        return t;
}

static inline uint64_t atomic64_read(const atomic64_t *v)
{
        uint64_t t;

        __asm__ volatile("ld%U1%X1 %0,%1" : "=r"(t) : "m"(v->counter));

        return t;
}

static inline void atomic64_set(atomic64_t *v, uint64_t i)
{
        __asm__ volatile("std%U0%X0 %1,%0" : "=m"(v->counter) : "r"(i));
}

static inline void atomic64_add(uint64_t a, atomic64_t *v)
{
        uint64_t t;

        __asm__ volatile(
"1:     ldarx   %0,0,%3         # atomic64_add\n\
        add     %0,%2,%0\n\
        stdcx.  %0,0,%3 \n\
        bne-    1b"
        : "=&r" (t), "+m" (v->counter)
        : "r" (a), "r" (&v->counter)
        : "cc");
}

static inline uint64_t atomic64_add_return(uint64_t a, atomic64_t *v)
{
        uint64_t t;

        __asm__ volatile(
"1:     ldarx   %0,0,%2         # atomic64_add_return\n\
        add     %0,%1,%0\n\
        stdcx.  %0,0,%2 \n\
        bne-    1b"
        : "=&r" (t)
        : "r" (a), "r" (&v->counter)
        : "cc", "memory");

        return t;
}

#define atomic64_add_negative(a, v)     (atomic64_add_return((a), (v)) < 0)

static inline void atomic64_sub(uint64_t a, atomic64_t *v)
{
        uint64_t t;

        __asm__ volatile(
"1:     ldarx   %0,0,%3         # atomic64_sub\n\
        subf    %0,%2,%0\n\
        stdcx.  %0,0,%3 \n\
        bne-    1b"
        : "=&r" (t), "+m" (v->counter)
        : "r" (a), "r" (&v->counter)
        : "cc");
}

static inline uint64_t atomic64_sub_return(uint64_t a, atomic64_t *v)
{
        uint64_t t;

        __asm__ volatile(
"1:     ldarx   %0,0,%2         # atomic64_sub_return\n\
        subf    %0,%1,%0\n\
        stdcx.  %0,0,%2 \n\
        bne-    1b"
        : "=&r" (t)
        : "r" (a), "r" (&v->counter)
        : "cc", "memory");

        return t;
}

static inline void atomic64_inc(atomic64_t *v)
{
        uint64_t t;

        __asm__ volatile(
"1:     ldarx   %0,0,%2         # atomic64_inc\n\
        addic   %0,%0,1\n\
        stdcx.  %0,0,%2 \n\
        bne-    1b"
        : "=&r" (t), "+m" (v->counter)
        : "r" (&v->counter)
        : "cc", "xer");
}

static inline uint64_t atomic64_inc_return(atomic64_t *v)
{
        uint64_t t;

        __asm__ volatile(
"1:     ldarx   %0,0,%1         # atomic64_inc_return\n\
        addic   %0,%0,1\n\
        stdcx.  %0,0,%1 \n\
        bne-    1b"
        : "=&r" (t)
        : "r" (&v->counter)
        : "cc", "xer", "memory");

        return t;
}

/*
 * atomic64_inc_and_test - increment and test
 * @v: pointer of type atomic64_t
 *
 * Atomically increments @v by 1
 * and returns true if the result is zero, or false for all
 * other cases.
 */
#define atomic64_inc_and_test(v) (atomic64_inc_return(v) == 0)

static inline void atomic64_dec(atomic64_t *v)
{
        uint64_t t;

        __asm__ volatile(
"1:     ldarx   %0,0,%2         # atomic64_dec\n\
        addic   %0,%0,-1\n\
        stdcx.  %0,0,%2\n\
        bne-    1b"
        : "=&r" (t), "+m" (v->counter)
        : "r" (&v->counter)
        : "cc", "xer");
}

static inline uint64_t atomic64_dec_return(atomic64_t *v)
{
        uint64_t t;

        __asm__ volatile(
"1:     ldarx   %0,0,%1         # atomic64_dec_return\n\
        addic   %0,%0,-1\n\
        stdcx.  %0,0,%1\n\
        bne-    1b"
        : "=&r" (t)
        : "r" (&v->counter)
        : "cc", "xer", "memory");

        return t;
}

#define atomic64_sub_and_test(a, v)     (atomic64_sub_return((a), (v)) == 0)
#define atomic64_dec_and_test(v)        (atomic64_dec_return((v)) == 0)

/*
 * Atomically test *v and decrement if it is greater than 0.
 * The function returns the old value of *v minus 1.
 */
static inline uint64_t atomic64_dec_if_positive(atomic64_t *v)
{
        uint64_t t;

        __asm__ volatile(
"1:     ldarx   %0,0,%1         # atomic64_dec_if_positive\n\
        addic.  %0,%0,-1\n\
        blt-    2f\n\
        stdcx.  %0,0,%1\n\
        bne-    1b"
        "\n\
2:"     : "=&r" (t)
        : "r" (&v->counter)
        : "cc", "xer", "memory");

        return t;
}

#define atomic64_cmpxchg(v, o, n) (cmpxchg(&((v)->counter), (o), (n)))
#define atomic64_xchg(v, new) (xchg(&((v)->counter), new))

/**
 * atomic64_add_unless - add unless the number is a given value
 * @v: pointer of type atomic64_t
 * @a: the amount to add to v...
 * @u: ...unless v is equal to u.
 *
 * Atomically adds @a to @v, so uint64_t as it was not @u.
 * Returns non-zero if @v was not @u, and zero otherwise.
 */
static inline uint32_t atomic64_add_unless(atomic64_t *v, uint64_t a, uint64_t u)
{
        uint64_t t;

        __asm__ volatile (
"1:     ldarx   %0,0,%1         # atomic_add_unless\n\
        cmpd    0,%0,%3 \n\
        beq-    2f \n\
        add     %0,%2,%0 \n"
"       stdcx.  %0,0,%1 \n\
        bne-    1b \n"
"       subf    %0,%2,%0 \n\
2:"
        : "=&r" (t)
        : "r" (&v->counter), "r" (a), "r" (u)
        : "cc", "memory");

        return t != u;
}

#define atomic64_inc_not_zero(v) atomic64_add_unless((v), 1, 0)

#endif /* _ASM_POWERPC_ATOMIC_H_ */