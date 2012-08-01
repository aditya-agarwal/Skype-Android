#ifndef _SHA1_HPP
#define _SHA1_HPP

#include "def.h"

class SHA1 {
	uint state[5];
	uint count[2];
	uchar buffer[64];

	void transform(const uchar buf[64]);

	public:

	enum {BINARY_DIGEST_SIZE=20};

	void Reset(void);
	void Update(const void * const ptr,const uint len);
	SHA1(void) { Reset(); }
	SHA1(const void * const ptr,const uint len)
		{ Reset(); Update(ptr,len); }
	void GetBinary(uchar digest[20]);
	void GetAscii(char *digest);
	};

#endif //_SHA1_HPP
