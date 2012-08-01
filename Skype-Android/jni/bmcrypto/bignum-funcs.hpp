#ifndef __bignum_funcs_hpp__
#define __bignum_funcs_hpp__

#include "def.h"
#include <string.h>
#if defined(_MSC_VER) && defined(_ARM_) && !defined(NO_BIGNUM_INLINE_ASM)
#include <armintr.h>
#endif

struct bignum_funcs {
#ifdef _MSC_VER
typedef uint element_type;
typedef unsigned __int64 two_element_type;
#else
#ifdef __WATCOMC__
typedef ushort element_type;
typedef uint two_element_type;
#else
typedef uint element_type;
typedef unsigned long long two_element_type;
#endif
#endif

#if defined(__ARM_ARCH_3M__) || defined(__ARM_ARCH_4__) || \
	defined(__ARM_ARCH_4T__) || \
	defined(__ARM_ARCH_5__) || defined(__ARM_ARCH_5E__) || \
	defined(__ARM_ARCH_5T__) || defined(__ARM_ARCH_5TE__)
#ifndef __thumb__
#define BIGNUM_ARM_HAVE_LONG_MULTIPLY
#endif
#endif

#define ELEMENT_BITS			(sizeof(bignum_funcs::element_type)*8)
#define ELEMENT_ALL_BITS_SET	((element_type)(~(element_type)0))

	static void multiply_add_one_line(
							element_type * const dest,const element_type u,
						const element_type * const src,const uint n) throw();
			// n must not be zero
			// this will modify n+1 elements in dest[]
	static void do_multiply(const uint n,element_type * const dest,
		const element_type * const a,const element_type * const b) throw();
			// n must not be zero
	static uint is_less_than(const uint n,
		const element_type * const a,const element_type * const b) throw();
	static void mod_div(const uint n,element_type * const dest,
		element_type * const src,const element_type * const modulus) throw();
			// the high bit of modulus must be set
			// src < (modulus<<(n*ELEMENT_BITS))
			// will garble src
	static void mod_square(element_type * const x,
				const element_type * const modulus,
				element_type * const mult_result /* length 2*n */,
				element_type * const tarr /* length 2*n-2 */,
				const uint n /* must not be 0 */) throw();
	};

inline void bignum_funcs::multiply_add_one_line(element_type *dest,
				element_type u,const element_type *src,uint n) throw()
{		// n must not be zero
		// this will modify n+1 elements in dest[]

#if defined(__GNUC__) && defined(i386) && !defined(NO_BIGNUM_INLINE_ASM) && (__GNUC__ >= 3 || !defined(__OPTIMIZE__) /* gcc 2.95.4 does not compile it with optimize */)
	uint dummy1,saved_ebx;
	uint zerodummy1=0, zerodummy2=0;
	__asm__ __volatile__ (
#if defined(__PIC__) || defined(__pic__)
	"movl %%ebx,%3"					"\n"
#endif
	"testl $1,%7"					"\n"
	"jz 0f"							"\n"
	"movl (%4),%%eax"				"\n"
	"incl %1"						"\n"
	"xorl %%ebx,%%ebx"				"\n"
	"jmp 1f"						"\n"
	"0:movl (%4,%1,4),%%eax"		"\n"
	"movl %%edx,%%ebx"				"\n"
	"mull %6"						"\n"
	"addl %%eax,%%ebx"				"\n"
		"movl 4(%4,%1,4),%%eax"		"\n"
	"adcl $0,%%edx"					"\n"
	"addl %%ebx,(%5,%1,4)"			"\n"
		"movl %%edx,%%ebx"			"\n"
	"adcl $0,%%ebx"					"\n"
	"addl $2,%1"					"\n"
		"1:mull %6"					"\n"
		"addl %%eax,%%ebx"			"\n"
		"adcl $0,%%edx"				"\n"
		"addl %%ebx,-4(%5,%1,4)"	"\n"
		"adcl $0,%%edx"				"\n"
	"cmpl %7,%1"					"\n"
	"jb	0b"							"\n"
	"movl %%edx,(%5,%1,4)"			"\n"
#if defined(__PIC__) || defined(__pic__)
	"movl %3,%%ebx"					"\n"
#endif
		: "=&a"(dummy1),
#if __GNUC__ >= 3
					"+&r"
#else
					"+&c"	/* gcc 2.95.4 gets compilation error with "r" here */
#endif
						(zerodummy1), "+&d"(zerodummy2), "=m" (saved_ebx)
		: "SD"(src), "r"(dest), "rm" (u), "m" (n)
		: "memory", "cc"
#if !defined(__PIC__) && !defined(__pic__)
						, "ebx"
#endif
								);
#elif defined(__GNUC__) && defined(__arm__) && defined(BIGNUM_ARM_HAVE_LONG_MULTIPLY) && !defined(NO_BIGNUM_INLINE_ASM)
	uint carry=0;
	__asm__ __volatile__ (
		"tst	%3,#1"			"\n"
		"mov	%3,#0"			"\n"
		"beq	0f"				"\n"

		"ldr	r2,[%2],#4"		"\n"	// *dest++
		"ldr	r0,[%1],#4"		"\n"	// *src++
		"adds	r2,r2,%0"		"\n"
		"adc	%0,%3,%3"		"\n"
		"umlal	r2,%0,%4,r0"	"\n"	// %0:r2 += u * r0
		"str	r2,[%2,#-4]"	"\n"
		"cmp	%2,%5"			"\n"
		"bcs	1f"				"\n"

		"0:"
		"ldmia	%2!,{r2,r3}"	"\n"	// *dest++, *dest++
		"ldmia	%1!,{r0,r1}"	"\n"	// *src++, *src++
		"adds	r2,r2,%0"		"\n"
		"adc	%0,%3,%3"		"\n"
		"umlal	r2,%0,%4,r0"	"\n"	// %0:r2 += u * r0
		"str	r2,[%2,#-8]"	"\n"

		"adds	r3,r3,%0"		"\n"
		"adc	%0,%3,%3"		"\n"
		"umlal	r3,%0,%4,r1"	"\n"	// %0:r3 += u * r1
		"str	r3,[%2,#-4]"	"\n"

		"cmp	%2,%5"			"\n"
		"bcc	0b"				"\n"
		"1:str	%0,[%2]"		"\n"
		: "+&r"(carry), "+&r"(src), "+&r"(dest), "+&r"(n)
		: "r"(u), "r"(dest+n)
		: "r0", "r1", "r2", "r3", "memory", "cc");
#elif defined(_MSC_VER) && defined(_M_IX86) && !defined(NO_BIGNUM_INLINE_ASM)
	_asm { mov	esi,src				}
	_asm { xor	ebx,ebx				}
	_asm { mov	edi,dest			}
	_asm { xor	edx,edx				}
	_asm { test n,1					}
	_asm { jz	la1					}
	_asm { mov	eax,[esi]			}
	_asm { mov	ebx,1				}
	_asm { xor	ecx,ecx				}
	_asm { jmp	la2					}
	_asm { la1:mov eax,[esi+ebx*4]	}
	_asm { mov	ecx,edx				}
	_asm { mul	u					}
	_asm { add	ecx,eax				}
	_asm { mov	eax,[4+esi+ebx*4]	}
	_asm { adc	edx,0				}
	_asm { add	[edi+ebx*4],ecx		}
	_asm { mov	ecx,edx				}
	_asm { adc	ecx,0				}
	_asm { add	ebx,2				}
	_asm { la2:mul u				}
	_asm { add	ecx,eax				}
	_asm { adc	edx,0				}
	_asm { add	[edi-4+ebx*4],ecx	}
	_asm { adc	edx,0				}
	_asm { cmp	ebx,n				}
	_asm { jb	la1					}
	_asm { mov	[edi+ebx*4],edx		}
#else
		// writing this loop into assembler improves mod_exp() by ca 15%

		//!!! this triggers a pgcc-2.91.60 optimization bug with -O5 and above

	element_type carry=0;
	for (uint i=0;i < n;i++) {
		const two_element_type value=dest[i] + (two_element_type)carry +
										(u * (two_element_type)src[i]);
		dest[i]=(element_type)value;
		carry=(element_type)(value >> (sizeof(element_type)*8));
		}

	dest[n]=carry;
#endif
	}

inline void bignum_funcs::do_multiply(const uint n,element_type * const dest,
		const element_type * const a,const element_type * const b) throw()
{			// n must not be zero

	memset(dest,0,sizeof(*dest) * 2*n);

	for (uint i=0;i < n;i++)
		multiply_add_one_line(&dest[i],a[i],&b[0],n);
	}

inline uint bignum_funcs::is_less_than(const uint n,
		const element_type * const a,const element_type * const b) throw()
{
	for (uint i=n;i--;) {
		if (a[i] < b[i])
			return 1;
		if (a[i] > b[i])
			return 0;
		}
	return 0;
	}

inline void bignum_funcs::mod_div(const uint n,element_type * const dest,
		element_type * const src,const element_type * const modulus) throw()
{
		// dest: n elements
		// src: 2n elements (will be garbled)
		//
		// the high bit of modulus must be set
		// src < (modulus<<(n*ELEMENT_BITS))
		// will garble src

	element_type divisor_element=modulus[n-1];
	element_type divisor_inverse=0;
	if (divisor_element == ELEMENT_ALL_BITS_SET)
		divisor_element=0;
	  else {
		divisor_element++;

#if defined(__GNUC__) && defined(i386) && !defined(NO_BIGNUM_INLINE_ASM) && (__GNUC__ >= 3 || !defined(__OPTIMIZE__) /* gcc 2.96 -O1 forgets to load eax=divisor_inverse before asm */)
		{ uint dummy;
		__asm__ __volatile__ (
		"movl	%2,%%edx"	"\n"
		"negl	%%edx"		"\n"
		"divl	%2"			"\n"
		: "+&a"(divisor_inverse), "=&d"(dummy)
		: "rm"(divisor_element)
		: "cc"); }
#elif defined(_MSC_VER) && defined(_M_IX86) && !defined(NO_BIGNUM_INLINE_ASM)
		_asm { mov	edx,divisor_element }
		_asm { xor	eax,eax }
		_asm { neg	edx }
		_asm { div	divisor_element }
		_asm { mov	divisor_inverse,eax }
#else
			// this ulonglong division takes 3% of mod_exp() time on ARM920t
		divisor_inverse=(element_type)((((two_element_type)(
				ELEMENT_ALL_BITS_SET-(divisor_element-1))) << ELEMENT_BITS) /
															divisor_element);
#endif
		}

	{ element_type *src_p=src+n-1;
	do {

#if defined(__GNUC__) && defined(i386) && !defined(NO_BIGNUM_INLINE_ASM) && (__GNUC__ >= 3 || !defined(__OPTIMIZE__) /* gcc 2.96 -O1 forgets to reload registers between the two asm sequences here */)
		{ uint dummy1, dummy2, dummy3;
		uint q;
		__asm__ __volatile__ (
		"movl	%4,%1"			"\n"
		"cmpl	$0,%5"			"\n"
		"movl	(%1),%%eax"		"\n"
		"movl	%%eax,%3"		"\n"
		"je		0f"				"\n"

		"mull	%6"				"\n"
		"addl	%%edx,%3"		"\n"
		"movl	%3,%%eax"		"\n"
		"mull	%5"				"\n"
		"subl	-4(%1),%%eax"	"\n"
		"sbbl	(%1),%%edx"		"\n"
		"negl	%%eax"			"\n"
		"adcl	$0,%%edx"		"\n"
		"negl	%%edx"			"\n"
		"jz		3f"				"\n"
		"2:incl	%3"				"\n"
		"subl	%5,%%eax"		"\n"
		"sbbl	$0,%%edx"		"\n"
		"jnz	2b"				"\n"
		"3:cmpl	%5,%%eax"		"\n"
		"sbbl	$-1,%3"			"\n"
		"0:"					"\n"
		: "=&a"(dummy1), "=&r"(dummy2), "=&d"(dummy3), "=&r"(q)
		: "rm"(src_p+n), "rm"(divisor_element), "rm"(divisor_inverse)
		: "memory", "cc");

		uint zerodummy=0;
		element_type *src_ptr=src_p;
		const element_type *modulus_ptr=modulus;
		__asm__ __volatile__ (
		"0:movl	(%4),%%eax"		"\n"
		"movl	%%edx,%1"		"\n"
		"addl	$4,%3"			"\n"
		"addl	$4,%4"			"\n"
		"mull	%5"				"\n"
		"addl	%1,%%eax"		"\n"
		"adcl	$0,%%edx"		"\n"
		"subl	%%eax,-4(%3)"	"\n"
		"adcl	$0,%%edx"		"\n"
		"cmpl	%6,%3"			"\n"
		"jb		0b"				"\n"
		"subl	%%edx,(%3)"		"\n"
		: "=&a"(dummy1), "=&r"(dummy2), "+&d"(zerodummy),
										"+&r"(src_ptr), "+&r"(modulus_ptr)
		:
#if defined(__PIC__) || defined(__pic__)
			"rm"
#else
			"r"		// changing this to "rm" makes code slower
#endif
				(q), "rm"(src_p+n)
		: "memory", "cc"); }
#elif defined(_MSC_VER) && defined(_M_IX86) && !defined(NO_BIGNUM_INLINE_ASM)
		{ element_type * const last_elem_ptr=src_p+n;
		_asm { mov	ebx,last_elem_ptr }
		_asm { cmp	divisor_element,0 }
		_asm { mov	esi,src_p	}
		_asm { mov	edi,modulus	}
		_asm { mov	eax,[ebx]	}
		_asm { mov	ecx,eax		}
		_asm { je	lb1			}

		_asm { mul	divisor_inverse }
		_asm { add	ecx,edx		}
		_asm { mov	eax,ecx		}
		_asm { mul	divisor_element }
		_asm { sub	eax,[ebx-4]	}
		_asm { sbb	edx,[ebx]	}
		_asm { neg	eax			}
		_asm { adc	edx,0		}
		_asm { neg	edx			}
		_asm { jz	lb4			}
		_asm { lb3:inc ecx		}
		_asm { sub	eax,divisor_element }
		_asm { sbb	edx,0		}
		_asm { jnz	lb3			}
		_asm { lb4:cmp eax,divisor_element }
		_asm { sbb	ecx,-1		}

		_asm { lb1:xor edx,edx	}

		_asm { lb2:mov eax,[edi]	}
		_asm { mov	ebx,edx		}
		_asm { add	esi,4		}
		_asm { add	edi,4		}
		_asm { mul	ecx			}
		_asm { add	eax,ebx		}
		_asm { adc	edx,0		}
		_asm { sub	[esi-4],eax	}
		_asm { adc	edx,0		}
		_asm { cmp	esi,last_elem_ptr }
		_asm { jb	lb2			}
		_asm { sub	[esi],edx	}}
#elif defined(__GNUC__) && defined(__arm__) && defined(BIGNUM_ARM_HAVE_LONG_MULTIPLY) && !defined(NO_BIGNUM_INLINE_ASM)
		element_type q;
		{ uint dummy1,dummy2;
		__asm__ __volatile__ (

				// underestimate quotient q

			"ldmia	%3,{r0,r1}	\n"		// src_p[n-1], src_p[n]
			"cmp	%4,#0		\n"		// cmp divisor_element,0
			"mov	%2,r1		\n"
			"beq	1f			\n"

			"umull	%0,%2,r1,%5	\n"		// %2:%0 = q * divisor_inverse
			"add	%2,%2,r1	\n"

			"umull	%0,%1,%2,%4	\n"		// %1:%0 = q * divisor_element
			"subs	r0,r0,%0	\n"
			"sbcs	r1,r1,%1	\n"

			"beq	2f			\n"		// cmp high_low,divisor_element
			"0:subs	r0,r0,%4	\n"
			"add	%2,%2,#1	\n"
			"sbcs	r1,r1,#0	\n"
			"bne	0b			\n"
			"2:cmp	r0,%4		\n"
			"addcs	%2,%2,#1	\n"
			"1:					\n"
			: "=&r"(dummy1), "=&r"(dummy2), "=&r"(q)
			: "r"(src_p+n-1), "r"(divisor_element), "r"(divisor_inverse)
			: "r0", "r1", "memory", "cc"); }

		{ uint dummy1;
		element_type *src_ptr=src_p;
		const element_type *modulus_ptr=modulus;
		element_type carry=0;
		__asm__ __volatile__ (

				// subtract q*modulus from src_p[0..n]

			"tst	%6,#1"			"\n"
			"beq	0f"				"\n"

			"mov	%0,#0"			"\n"
			"ldr	r0,[%3],#4"		"\n"	// *modulus_ptr++
			"umlal	%1,%0,r0,%4"	"\n"	// %0:carry += r0 * q
			"ldr	r0,[%2],#4"		"\n"	// *src_ptr++
			"subs	r0,r0,%1"		"\n"
			"mov	%1,%0"			"\n"
			"str	r0,[%2,#-4]"	"\n"	// *(src_ptr-1)
			"addcc	%1,%1,#1"		"\n"
			"cmp	%2,%5"			"\n"	// src_ptr,last_src
			"bhi	1f"				"\n"

			"0:"
			"mov	%0,#0"			"\n"
			"ldmia	%3!,{r0,r1}"	"\n"	// *modulus_ptr++, *modulus_ptr++
			"umlal	%1,%0,r0,%4"	"\n"	// %0:carry += r0 * q
			"ldmia	%2!,{r0,r2}"	"\n"	// *src_ptr++, *src_ptr++
			"subs	r0,r0,%1"		"\n"
			"addcc	%0,%0,#1"		"\n"

			"mov	%1,#0"			"\n"
			"umlal	%0,%1,r1,%4"	"\n"	// %1:carry += r1 * q
			"subs	r2,r2,%0"		"\n"
			"stmdb	%2,{r0,r2}"		"\n"	// *(src_ptr-2)
			"addcc	%1,%1,#1"		"\n"

			"cmp	%2,%5"			"\n"	// src_ptr,last_src
			"bls	0b"				"\n"
			"1:ldr	r0,[%2]"		"\n"
			"sub	r0,r0,%1"		"\n"
			"str	r0,[%2]"		"\n"
			: "=&r"(dummy1), "+&r"(carry), "+&r"(src_ptr), "+&r"(modulus_ptr)
			: "r"(q), "r"(src_p+n-1), "r"(n)
			: "r0", "r1", "r2", "memory", "cc"); }
#else
			// underestimate quotient q
			//   writing this into assembler improves mod_exp() by 16%

		element_type q=src_p[n];
		if (divisor_element) {
#if 1
			{ two_element_type high_low=(((two_element_type)q) <<
												ELEMENT_BITS) + src_p[n-1];
			q+=(element_type)((q *
						(two_element_type)divisor_inverse) >> ELEMENT_BITS);

			high_low-=q * (two_element_type)divisor_element;
			while (high_low >= divisor_element) {
				high_low -= divisor_element;
				q++;
				}}
#elif 0
				// the most basic version
			{ const two_element_type high_low=(((two_element_type)q) <<
												ELEMENT_BITS) + src_p[n-1];
			q=(element_type)(high_low / divisor_element); }
#elif 0
				// this version could be faster on PowerPC

			{ const element_type d1=divisor_element >> (ELEMENT_BITS/2);
			const element_type d0=divisor_element & ((1U<<(ELEMENT_BITS/2))-1);

			element_type q1=q / d1;
			element_type r1=q - q1*d1;
			{ const element_type m=q1*d0;
			r1=(r1 << (ELEMENT_BITS/2)) | (src_p[n-1] >> (ELEMENT_BITS/2));
			if (r1 < m) {
				q1--;
				r1+=divisor_element;
				if (r1 >= divisor_element) /* i.e. we didn't get carry when adding to r1 */
					if (r1 < m) {
						q1--;
						r1+=divisor_element;
						}
				}
			r1-=m; }

			element_type q0=r1/d1;
			element_type r0=r1 - q0*d1;
			{ const element_type m=q0*d0;
			r0=(r0 << (ELEMENT_BITS/2)) | (src_p[n-1] & ((1U<<(ELEMENT_BITS/2))-1));
			if (r0 < m) {
				q0--;
				r0+=divisor_element;
				if (r0 >= divisor_element)
					if (r0 < m)
						q0--;
				}}

			q=(q1 << (ELEMENT_BITS/2)) | q0; }
#endif
			}

			// subtract q*modulus from src_p[0..n]
			//   writing this loop into assembler improves mod_exp() by 1.6x

		{ element_type carry=0;
		for (uint i=0;i < n;i++) {
			two_element_type value=modulus[i] * (two_element_type)q + carry;
			carry=(element_type)(value >> ELEMENT_BITS);

			value=(src_p[i] | (((two_element_type)1U) << ELEMENT_BITS)) -
								(element_type)(value & ELEMENT_ALL_BITS_SET);
			src_p[i]=(element_type)(value & ELEMENT_ALL_BITS_SET);
			carry+=1-((element_type)(value >> ELEMENT_BITS));
			}
		src_p[n]-=carry; }
#endif

			// correct estimate

		while ((
#if defined(_MSC_VER) && defined(_ARM_) && !defined(NO_BIGNUM_INLINE_ASM)
				(volatile element_type *)	// otherwise MSVC does not
											//   realise we are modifying
											//   src in asm code
#endif
						src_p)[n] || !is_less_than(n,src_p,&modulus[0])) {
#if defined(__GNUC__) && defined(i386) && !defined(NO_BIGNUM_INLINE_ASM)
			{ uint dummy1,dummy2,dummy3;
			__asm__ __volatile__ (
			"xorl	%1,%1"			"\n"	// gcc 2.96 produces wrong code if this is implemented using zerodummy instead
			"xorl	%2,%2"			"\n"
			"0:movl	(%3,%1,4),%0"	"\n"
			"subl	%2,%0"			"\n"
			"movl	$0,%2"			"\n"
			"adcl	%2,%2"			"\n"
			"subl	(%4,%1,4),%0"	"\n"
			"adcl	$0,%2"			"\n"
			"movl	%0,(%3,%1,4)"	"\n"
			"incl	%1"				"\n"
			"cmpl	%5,%1"			"\n"
			"jb		0b"				"\n"
			"subl	%2,(%3,%1,4)"	"\n"
			: "=&r"(dummy1), "=&r"(dummy2), "=&r"(dummy3)
			: "r"(src_p), "r"(modulus),
#if defined(__PIC__) || defined(__pic__)
										"m"
#else
										"rm"
#endif
											(n)
			: "memory", "cc"); }
#elif defined(_MSC_VER) && defined(_M_IX86) && !defined(NO_BIGNUM_INLINE_ASM)
			_asm { mov	esi,src_p			}
			_asm { xor	ecx,ecx				}
			_asm { mov	edi,modulus			}
			_asm { xor	edx,edx				}
			_asm { lc1:mov eax,[esi+ecx*4]	}
			_asm { sub	eax,edx				}
			_asm { mov	edx,0				}
			_asm { adc	edx,edx				}
			_asm { sub	eax,[edi+ecx*4]		}
			_asm { adc	edx,0				}
			_asm { mov	[esi+ecx*4],eax		}
			_asm { inc	ecx					}
			_asm { cmp	ecx,n				}
			_asm { jb	lc1					}
			_asm { sub	[esi+ecx*4],edx		}
#elif defined(__GNUC__) && defined(__arm__) && defined(BIGNUM_ARM_HAVE_LONG_MULTIPLY) && !defined(NO_BIGNUM_INLINE_ASM)
		{ element_type *src_ptr=src_p;
		const element_type *modulus_ptr=modulus;
		const element_type * const end_src=src_p + n;
		__asm__ __volatile__ (
			"tst	%3,#1		\n"
			"beq	2f			\n"

			"ldr	r0,[%0],#4	\n"		// *src_ptr++
			"ldr	r2,[%1],#4	\n"		// *modulus_ptr++
			"subs	r0,r0,r2	\n"
			"str	r0,[%0,#-4]	\n"
			"teq	%0,%2		\n"		// src_ptr, end_src
			"bne	0f			\n"
			"b		1f			\n"

			"2:subs	r0,r0,#0	\n"		// just to init carry flag

			"0:ldmia %0!,{r0,r1}\n"		// *src_ptr++
			"ldmia	%1!,{r2,r3}	\n"		// *modulus_ptr++
			"sbcs	r0,r0,r2	\n"
			"sbcs	r1,r1,r3	\n"
			"stmdb	%0,{r0,r1}	\n"		// *(src_ptr-2)
			"teq	%0,%2		\n"		// src_ptr, end_src
			"bne	0b			\n"

			"1:ldr	r0,[%0]		\n"		// src_ptr
			"sbc	r0,r0,#0	\n"
			"str	r0,[%0]		\n"
			: "+&r"(src_ptr), "+&r"(modulus_ptr)
			: "r"(end_src), "r"(n)
			: "r0", "r1", "r2", "r3", "memory", "cc"); }
#elif defined(_MSC_VER) && defined(_ARM_) && !defined(NO_BIGNUM_INLINE_ASM)
			__gregister_set(3,(uint)(src_p + n));
			__gregister_set(2,(uint)(modulus));
			__gregister_set(1,(uint)(src_p));
			__gregister_set(0,n);

			__emit(0xe92d007f);		//	stmdb	sp!,{r0-r6}	// save regs

			__emit(0xe3100001);		//	tst		r0,#1
			__emit(0x0a000006);		//	beq		2f

			__emit(0xe4910004);		//	ldr		r0,[r1],#4	// *src_ptr++
			__emit(0xe4925004);		//	ldr		r5,[r2],#4	// *modulus_ptr++
			__emit(0xe0500005);		//	subs	r0,r0,r5
			__emit(0xe5010004);		//	str		r0,[r1,#-4]
			__emit(0xe1310003);		//	teq		r1,r3		// src_ptr, end_src
			__emit(0x1a000001);		//	bne		0f
			__emit(0xea000007);		//	b		1f

			__emit(0xe2500000);		//	2:subs	r0,r0,#0	// just to init carry flag

			__emit(0xe8b10011);		//	0:ldmia r1!,{r0,r4}	// *src_ptr++
			__emit(0xe8b20060);		//	ldmia	r2!,{r5,r6}	// *modulus_ptr++
			__emit(0xe0d00005);		//	sbcs	r0,r0,r5
			__emit(0xe0d44006);		//	sbcs	r4,r4,r6
			__emit(0xe9010011);		//	stmdb	r1,{r0,r4}	// *(src_ptr-2)
			__emit(0xe1310003);		//	teq		r1,r3		// src_ptr, end_src
			__emit(0x1afffff8);		//	bne		0b

			__emit(0xe5910000);		//	1:ldr	r0,[r1]		// src_ptr
			__emit(0xe2c00000);		//	sbc		r0,r0,#0
			__emit(0xe5810000);		//	str		r0,[r1]

			__emit(0xe8bd007f);		//	ldmia	sp!,{r0-r6}	// restore regs
#else
				// writing this loop into asm improves mod_exp() by i386:4% ARM:10%

			element_type carry=0;
			for (uint i=0;i < n;i++) {
				const element_type modulus_element=modulus[i];
				if (!src_p[i] && carry)
					src_p[i]=ELEMENT_ALL_BITS_SET - modulus_element;
				  else {
					src_p[i]-=carry;
					carry=(src_p[i] < modulus_element);
					src_p[i]-=modulus_element;
					}
				}
			src_p[n]-=carry;
#endif
			}
		} while ((src_p--)-src > 0); }

	for (uint i=0;i < n;i++)
		dest[i]=src[i];
	}

inline void bignum_funcs::mod_square(element_type * const x,
				const element_type * const modulus,
				element_type * const mult_result /* length 2*n */,
				element_type *tarr /* length 2*n-2 */,
				const uint n /* must not be 0 */) throw()
{
		// set mult_result = x * x

	/*	GMP squaring (mpn_sqr_basecase):

			for (i=0;i < n;i++)
				prodp[2*i .. (2*i+1)] = x[i] * x[i]

			tarr[..] = 0

			for (i=1;i < n;i++)
				tarr[2*i-2 .. (n+i-2)] += x[i-1] * x[i..]

			tarr len is 2*n-2 now
			prodp len is 2*n now

			prodp[1..] += 2 * tarr

			total: ca n*(n/2) multiplications
		*/

	{ for (uint i=0;i < n;i++) {	// 7 us
		const element_type u=x[i];
		const two_element_type value=u * (two_element_type)u;

		mult_result[2*i  ]=(element_type)value;
		mult_result[2*i+1]=(element_type)(value >> ELEMENT_BITS);
		}}

	memset(tarr,0,(2*n-2)*sizeof(*tarr));	// 1 us

	{ for (uint i=1;i < n;i++)		// 66/227 us with/without asm
		multiply_add_one_line(&tarr[2*i-2],x[i-1],&x[i],n-i); }

#if defined(__GNUC__) && defined(i386) && !defined(NO_BIGNUM_INLINE_ASM)
	{ uint dummy1,dummy2,dummy3,saved_ebx;
	__asm__ __volatile__ (
#if defined(__PIC__) || defined(__pic__)
	"movl	%%ebx,%3"		"\n"
#endif
	"xorl	%1,%1"			"\n"	// gcc 2.96 produces wrong code if this is implemented using zerodummy instead
	"xorl	%2,%2"			"\n"
	"jmp	1f"				"\n"

	"0:movl	-4(%4,%1,8),%0"	"\n"
	"addl	%2,%0"			"\n"
	"movl	$0,%2"			"\n"
	"adcl	%2,%2"			"\n"
	"addl	-8(%5,%1,8),%0"	"\n"
	"adcl	$0,%2"			"\n"
	"addl	-8(%5,%1,8),%0"	"\n"
	"adcl	$0,%2"			"\n"
	"movl	%0,-4(%4,%1,8)"	"\n"

	"movl	(%4,%1,8),%%ebx" "\n"
	"addl	%2,%%ebx"		"\n"
	"movl	$0,%2"			"\n"
	"adcl	%2,%2"			"\n"
	"addl	-4(%5,%1,8),%%ebx" "\n"
	"adcl	$0,%2"			"\n"
	"addl	-4(%5,%1,8),%%ebx" "\n"
	"adcl	$0,%2"			"\n"
	"movl	%%ebx,(%4,%1,8)" "\n"

	"1:incl	%1"				"\n"
	"cmpl	%6,%1"			"\n"
	"jb		0b"				"\n"
	"addl	%2,-4(%4,%1,8)"	"\n"
#if defined(__PIC__) || defined(__pic__)
	"movl	%3,%%ebx"		"\n"
#endif
		: "=&r"(dummy1), "=&r"(dummy2), "=&r"(dummy3), "=m"(saved_ebx)
		: "r"(mult_result), "r"(tarr), "rm" (n)
		: "memory", "cc"
#if !defined(__PIC__) && !defined(__pic__)
						, "ebx"
#endif
								); }
#elif defined(_MSC_VER) && defined(_M_IX86) && !defined(NO_BIGNUM_INLINE_ASM)
	_asm { xor	ebx,ebx				}
	_asm { xor	ecx,ecx				}
	_asm { mov	esi,mult_result		}
	_asm { mov	edi,tarr			}
	_asm { jmp	ld2					}

	_asm { ld1:mov eax,[esi-4+ebx*8] }
	_asm { add	eax,ecx				}
	_asm { mov	ecx,0				}
	_asm { adc	ecx,ecx				}
	_asm { add	eax,[edi-8+ebx*8]	}
	_asm { adc	ecx,0				}
	_asm { add	eax,[edi-8+ebx*8]	}
	_asm { adc	ecx,0				}
	_asm { mov	[esi-4+ebx*8],eax	}

	_asm { mov	edx,[esi+ebx*8]		}
	_asm { add	edx,ecx				}
	_asm { mov	ecx,0				}
	_asm { adc	ecx,ecx				}
	_asm { add	edx,[edi-4+ebx*8]	}
	_asm { adc	ecx,0				}
	_asm { add	edx,[edi-4+ebx*8]	}
	_asm { adc	ecx,0				}
	_asm { mov	[esi+ebx*8],edx		}

	_asm { ld2:inc ebx				}
	_asm { cmp	ebx,n				}
	_asm { jb	ld1					}
	_asm { add	[esi-4+ebx*8],ecx	}
#elif defined(__GNUC__) && defined(__arm__) && defined(BIGNUM_ARM_HAVE_LONG_MULTIPLY) && !defined(NO_BIGNUM_INLINE_ASM)
		{ element_type carry=0;
		element_type *mult_result_ptr=mult_result+1;
		__asm__ __volatile__ (
			"cmp	%2,%3		\n"		// tarr,end_tarr
			"bcs	1f			\n"

			"0:ldmia %1!,{r0,r1}\n"		// mult_result_ptr
			"adds	r0,r0,%0	\n"
			"mov	%0,#0		\n"
			"ldmia	%2!,{r2,r3}	\n"		// tarr

			"adc	%0,%0,#0	\n"
			"adds	r0,r0,r2	\n"
			"adc	%0,%0,#0	\n"
			"adds	r0,r0,r2	\n"
			"adc	%0,%0,#0	\n"

			"adds	r1,r1,%0	\n"
			"mov	%0,#0		\n"
			"adc	%0,%0,#0	\n"
			"adds	r1,r1,r3	\n"
			"adc	%0,%0,#0	\n"
			"adds	r1,r1,r3	\n"
			"adc	%0,%0,#0	\n"

			"cmp	%2,%3		\n"		// tarr,end_tarr
			"stmdb	%1,{r0,r1}	\n"		// mult_result_ptr
			"bcc	0b			\n"

			"1:ldr	r0,[%1]		\n"		// mult_result_ptr
			"add	r0,r0,%0	\n"
			"str	r0,[%1]		\n"
			: "+&r"(carry), "+&r"(mult_result_ptr), "+&r"(tarr)
			: "r"(tarr+2*n-2)
			: "r0", "r1", "r2", "r3", "memory", "cc"); }
#else
		// writing this loop into assembler improves mod_exp() by 3%
	{ element_type carry=0;			// 29 us
	for (uint i=0;i < 2*n-2;i++) {
		const two_element_type value=mult_result[1+i] +
							(tarr[i] + (two_element_type)tarr[i]) + carry;
		mult_result[1+i]=(element_type)value;
		carry=(element_type)(value >> ELEMENT_BITS);
		}
	mult_result[2*n-1]+=(element_type)carry; }
#endif

		// now do the mod_div

	mod_div(n,x,mult_result,modulus);	// with/without asm: i386 133/353 us; ARM 2700/6400 us
	}

#endif
