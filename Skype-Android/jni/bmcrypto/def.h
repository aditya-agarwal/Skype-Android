#ifndef _DEF_H_
#define _DEF_H_

#ifdef _SYMBIAN
#include <e32base.h>
#endif

#if !(__BCPLUSPLUS__ > 0x310)
#ifndef _MSC_VER
#ifndef __GNUC__
#ifndef __WATCOMC__
#ifndef _SYMBIAN
#pragma pack(1)                 // for qc2
#pragma check_stack(off)        // for qc2
#endif
#endif
#endif
#endif
#endif

#ifndef NULL
#define NULL                0
#endif

typedef unsigned int        uint;
typedef unsigned short      ushort;
typedef unsigned long       ulong;
typedef unsigned char       uchar;

typedef signed int          sint;
typedef signed short        sshort;
typedef signed long         slong;
typedef signed char         schar;

#if defined(_MSC_VER) || defined(__WATCOM_INT64__)
typedef unsigned __int64	ulonglong;
typedef signed __int64		slonglong;
#define LONGLONG_CONST(n)	n
#define ULONGLONG_CONST(n)	n##U
#define LLPFX				"I64"
#else
#ifndef __WATCOMC__
typedef unsigned long long  ulonglong;
typedef signed long long    slonglong;
#define LONGLONG_CONST(n)	n##LL
#define ULONGLONG_CONST(n)	n##ULL
#define LLPFX				"ll"
#endif
#endif
typedef ulonglong uint64;
typedef slonglong int64;

#ifdef __WATCOMC__
void breakpoint(void);
#pragma aux breakpoint="int 3"

#pragma warning 389 4
#pragma warning 007 4
#pragma warning 604 4
#pragma warning 594 4
#pragma warning 391 4
#pragma warning 364 4
#pragma warning 297 4
#pragma warning 549 4
#pragma warning 188 4
#pragma warning 726 2
#pragma warning 737 2

#if __WATCOMC__ < 110
#ifndef _WATCOMC_BOOL_DEFINED
typedef unsigned char bool;
#define _WATCOMC_BOOL_DEFINED
#endif
#endif

#undef true
#define true ((bool)1)
#undef false
#define false ((bool)0)

#endif

#ifdef __GNUC__
#ifndef __EMX__
#define stricmp strcasecmp
#define strnicmp strncasecmp
int memicmp(const void *,const void *,uint);	// defined in Misc.cpp
#endif
#define __cdecl
//#define __cdecl		__attribute__((cdecl))
#define __stdcall	__attribute__((stdcall))
#define PACKED		__attribute__((packed))
#else
#define PACKED
#endif

#ifdef _SYMBIAN
#define stricmp strcasecmp
#define strnicmp strncasecmp
int memicmp(const void *,const void *,uint); // defined in Misc.cpp
#endif

#ifdef _WIN32_WCE
#define stricmp _stricmp
#define strnicmp _strnicmp
#define memicmp _memicmp
#define strdup _strdup
#endif

#ifndef __386__
#if !(__BCPLUSPLUS__ > 0x310)

#define FP_SEG(fp)          (*((uint *)&(fp) + 1))
#define FP_OFF(fp)          (*((uint *)&(fp)))
#define MK_FP(ptr,seg)      FP_SEG(ptr)=seg; FP_OFF(ptr)=0

#define BIOS_TIMER          (*(ulong far *)0x46c)
#define BIOS_KEYBOARD_FLAGS (*(uchar far *)0x417)

#endif
#endif

#define zero(v)             memset(v,0,sizeof(v))
#define __zero__(v)			__memset__(v,0,sizeof(v))
#define lenof(t)            (sizeof(t)/sizeof(*t))

#ifdef _MSC_VER
#define MSVC_cdecl __cdecl
#else
#define MSVC_cdecl
#if !(__BCPLUSPLUS__ > 0x310) 
#define bound(v,a,b)        if ((v)<(a)) (v)=(a);else if ((v)>(b)) (v)=(b);
#endif
#endif

#undef max
#undef min
#define max(a,b)            ((a)>=(b)?(a):(b))
#define min(a,b)            ((a)<=(b)?(a):(b))

#define NIL						((uint)~0)
#define NIL64					((uint64)~0)
#define PI						((double)3.1415926535897932)
#define DEGREES(deg)			(PI/180*(deg))
#define APPROXIMATE_COS(rad)	(1-rad*rad/2.0+(double)rad*rad*rad*rad/24.0)
#define APPROXIMATE_TAN(rad)	(rad + (double)rad*rad*rad/3.0 + (double)rad*rad*rad*rad*rad/7.5)
#endif
