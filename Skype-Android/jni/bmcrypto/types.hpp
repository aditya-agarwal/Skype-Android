/* ====================================================================
 * Basic integer type definitions
 * Written by Priit Kasesalu (priit@bluemoon.ee)
 * Confidential and proprietary
 */

#ifndef _TYPES_HPP_INCLUDED
#define _TYPES_HPP_INCLUDED

#include "def.h"

#if defined(__arm__)
#undef ARM
#define ARM
#endif

#if !defined(DATA_LITTLEENDIAN) && !defined(DATA_BIGENDIAN)
	#if defined(_M_I86) || defined(_M_IX86) || defined(__TURBOC__) || defined(__OS2__) || defined(ARM) || defined(i386)
		#define DATA_LITTLEENDIAN
	#elif defined(AMIGA) || defined(SYMANTEC_C) || defined(__APPLE__)
		#define DATA_BIGENDIAN
	#elif defined(__GNUC__) && defined(BYTES_BIG_ENDIAN)
		#define DATA_BIGENDIAN
	#elif defined(__GNUC__) && !defined(BYTES_BIG_ENDIAN)
		#define DATA_LITTLEENDIAN
	//#elif (((ushort)('12') >> 8) == '2')
	//	#define DATA_LITTLEENDIAN
	//#elif (((ushort)('12') >> 8) == '1')
	//	#define DATA_BIGENDIAN
	#else
		#error Cannot determine processor endianness
	#endif
#endif

typedef unsigned char lu8;
typedef unsigned char bu8;

#if defined(DATA_BIGENDIAN)

// big-endian variables (big-endian system)

typedef unsigned short bu16;
typedef unsigned int bu32;
typedef ulonglong bu64;

#endif

// little-endian variables (big-endian system)

#if defined(DATA_BIGENDIAN) || defined(ARM)

struct lu32 {
	unsigned char Val[4];

	inline operator uint(void) const {return ((uint)Val[3]<<24)|((uint)Val[2]<<16)|((uint)Val[1]<<8)|Val[0];}
	inline void operator =(uint u){
		Val[0]=u>>0;Val[1]=u>>8;Val[2]=u>>16;Val[3]=u>>24;}
} PACKED;

struct lu16 {
	unsigned char Val[2];

	inline operator uint(void) const {return ((uint)Val[1]<<8)|Val[0];}
	inline void operator =(uint u){
		Val[0]=u>>0;Val[1]=u>>8;}
} PACKED;

struct lu64 {
	unsigned char Val[8];

	inline operator ulonglong(void) const {return (((ulonglong)(*(lu32*)(Val+4)))<<32)+*(lu32*)Val;}
	inline void operator =(ulonglong u){*(lu32*)(Val+4)=(uint)(u>>32);*(lu32*)Val=(uint)u;}
} PACKED;

#endif

// little-endian variables (little-endian system)

#if defined(DATA_LITTLEENDIAN) && !defined(ARM)

typedef unsigned short lu16;
typedef unsigned int lu32;
typedef ulonglong lu64;

#endif

// big-endian variables (little-endian system)

#if defined(DATA_LITTLEENDIAN)

struct bu32 {
	unsigned char Val[4];

	inline bu32(void){}
	inline bu32(uint u){Val[0]=u>>24;Val[1]=u>>16;Val[2]=u>>8;Val[3]=u>>0;}
	inline operator uint(void) const {return ((uint)Val[0]<<24)|((uint)Val[1]<<16)|((uint)Val[2]<<8)|Val[3];}
	inline void operator =(uint u){Val[0]=u>>24;Val[1]=u>>16;Val[2]=u>>8;Val[3]=u>>0;}
} PACKED;

struct bu16 {
	unsigned char Val[2];

	inline bu16(void){}
	inline bu16(uint u){Val[0]=u>>8;Val[1]=u>>0;}
	inline operator uint(void) const {return ((uint)Val[0]<<8)|Val[1];}
	inline void operator =(uint u){Val[0]=u>>8;Val[1]=u>>0;}
} PACKED;

struct bu64 {
	unsigned char Val[8];

	inline bu64(void){}
	inline bu64(ulonglong u){*(bu32*)Val=(uint)(u>>32);*(bu32*)(Val+4)=(uint)u;}
	inline operator ulonglong(void) const {return (((ulonglong)(*(bu32*)Val))<<32)+*(bu32*)(Val+4);}
	inline void operator =(ulonglong u){*(bu32*)Val=(uint)(u>>32);*(bu32*)(Val+4)=(uint)u;}
} PACKED;

#endif

#endif // _TYPES_HPP_INCLUDED
