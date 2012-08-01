#ifndef _CRC_HPP_INCLUDED
#define _CRC_HPP_INCLUDED

#include "def.h"

class CRC8{
	uchar Crc;
	static const uchar Table[256];
public:
	void Reset(void){Crc=0;}
	CRC8(void){Reset();}
	bool Check(void) const {return !Crc;}
	void Update(uchar c){Crc=Table[c^Crc];}
    void Update(const char *s);
    void Update(const void *s,int len){const uchar *ss=(const uchar *)s;	
								while(len--) Crc=Table[*(ss++)^Crc];}
	operator uchar(void) const {return Crc;}
};

class CRC16 {
	ushort Crc;
	static const ushort Table[256];
public:
	void Reset(void){Crc=0;}
	CRC16(void){Reset();}
	bool Check(void) const {return !Crc;}
	void Update(uchar c){Crc=Table[(Crc>>8)^c]^(Crc<<8);}
    void Update(const char *s);
    void Update(const void *s,int len);
	operator ushort(void) const {return Crc;}
};

class CRC32 {
	uint Crc;
	static const uint Table[256];
public:
	void Reset(void){Crc=0xffffffff;}
	CRC32(void){Reset();}
	bool Check(void) const {return Crc==0xdebb20e3;}
	void Update(uchar c){Crc=Table[(Crc^c)&255]^(Crc>>8);}
	void Update(const char *s);
	void Update(const void *s,int len);
	void Update32(uint value);
	operator uint(void) const {return Crc;}
};

#endif // _CRC_HPP_INCLUDED
