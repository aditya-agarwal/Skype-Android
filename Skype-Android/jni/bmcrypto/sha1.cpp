#include "sha1.hpp"
#include <string.h>

/* Based on public domain implementation by Steve Reid <steve@edmweb.com>

Test Vectors (from FIPS PUB 180-1)
"abc"
	A9993E36 4706816A BA3E2571 7850C26C 9CD0D89D
"abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq"
	84983E44 1C3BD26E BAAE4AA1 F95129E5 E54670F1
A million repetitions of "a"
	34AA973C D4C4DAA4 F61EEB2B DBAD2731 6534016F
*/

#define rol(value,bits) (((value) << (bits)) | (((value)&0xffffffffU) >> (32 - (bits))))

#define blk(i) (block[i&15] = rol(block[(i+13)&15] ^ block[(i+8)&15] ^ block[(i+2)&15] ^ block[i&15], 1))

// (R0+R1), R2, R3, R4 are the different operations used in SHA1
#define R0(v,w,x,y,z,i) z+=((w&(x^y))^y)+  block[i]+0x5A827999+rol(v,5); w=rol(w,30);
#define R1(v,w,x,y,z,i) z+=((w&(x^y))^y)+    blk(i)+0x5A827999+rol(v,5); w=rol(w,30);
#define R2(v,w,x,y,z,i) z+=(w^x^y)+          blk(i)+0x6ED9EBA1+rol(v,5); w=rol(w,30);
#define R3(v,w,x,y,z,i) z+=(((w|x)&y)|(w&x))+blk(i)+0x8F1BBCDC+rol(v,5); w=rol(w,30);
#define R4(v,w,x,y,z,i) z+=(w^x^y)+          blk(i)+0xCA62C1D6+rol(v,5); w=rol(w,30);

void SHA1::transform(const uchar buf[64])
{
		// convert buf[] uchars into block[] uint's

	uint block[16];
	for (uint i=0;i < lenof(block);i++)
		block[i]=(uint)buf[i*4 +3] +
				((uint)buf[i*4 +2] <<  8) +
				((uint)buf[i*4 +1] << 16) +
				((uint)buf[i*4   ] << 24);

		// copy state[] to working vars

	uint a=state[0];
	uint b=state[1];
	uint c=state[2];
	uint d=state[3];
	uint e=state[4];

		// 4 rounds of 20 operations each

	R0(a,b,c,d,e, 0); R0(e,a,b,c,d, 1); R0(d,e,a,b,c, 2); R0(c,d,e,a,b, 3);
	R0(b,c,d,e,a, 4); R0(a,b,c,d,e, 5); R0(e,a,b,c,d, 6); R0(d,e,a,b,c, 7);
	R0(c,d,e,a,b, 8); R0(b,c,d,e,a, 9); R0(a,b,c,d,e,10); R0(e,a,b,c,d,11);
	R0(d,e,a,b,c,12); R0(c,d,e,a,b,13); R0(b,c,d,e,a,14); R0(a,b,c,d,e,15);
	R1(e,a,b,c,d,16); R1(d,e,a,b,c,17); R1(c,d,e,a,b,18); R1(b,c,d,e,a,19);
	R2(a,b,c,d,e,20); R2(e,a,b,c,d,21); R2(d,e,a,b,c,22); R2(c,d,e,a,b,23);
	R2(b,c,d,e,a,24); R2(a,b,c,d,e,25); R2(e,a,b,c,d,26); R2(d,e,a,b,c,27);
	R2(c,d,e,a,b,28); R2(b,c,d,e,a,29); R2(a,b,c,d,e,30); R2(e,a,b,c,d,31);
	R2(d,e,a,b,c,32); R2(c,d,e,a,b,33); R2(b,c,d,e,a,34); R2(a,b,c,d,e,35);
	R2(e,a,b,c,d,36); R2(d,e,a,b,c,37); R2(c,d,e,a,b,38); R2(b,c,d,e,a,39);
	R3(a,b,c,d,e,40); R3(e,a,b,c,d,41); R3(d,e,a,b,c,42); R3(c,d,e,a,b,43);
	R3(b,c,d,e,a,44); R3(a,b,c,d,e,45); R3(e,a,b,c,d,46); R3(d,e,a,b,c,47);
	R3(c,d,e,a,b,48); R3(b,c,d,e,a,49); R3(a,b,c,d,e,50); R3(e,a,b,c,d,51);
	R3(d,e,a,b,c,52); R3(c,d,e,a,b,53); R3(b,c,d,e,a,54); R3(a,b,c,d,e,55);
	R3(e,a,b,c,d,56); R3(d,e,a,b,c,57); R3(c,d,e,a,b,58); R3(b,c,d,e,a,59);
	R4(a,b,c,d,e,60); R4(e,a,b,c,d,61); R4(d,e,a,b,c,62); R4(c,d,e,a,b,63);
	R4(b,c,d,e,a,64); R4(a,b,c,d,e,65); R4(e,a,b,c,d,66); R4(d,e,a,b,c,67);
	R4(c,d,e,a,b,68); R4(b,c,d,e,a,69); R4(a,b,c,d,e,70); R4(e,a,b,c,d,71);
	R4(d,e,a,b,c,72); R4(c,d,e,a,b,73); R4(b,c,d,e,a,74); R4(a,b,c,d,e,75);
	R4(e,a,b,c,d,76); R4(d,e,a,b,c,77); R4(c,d,e,a,b,78); R4(b,c,d,e,a,79);

		// add the working vars back into state[]

	state[0]+=a;
	state[1]+=b;
	state[2]+=c;
	state[3]+=d;
	state[4]+=e;

		// wipe variables

	a=b=c=d=e=0;
	zero(block);
	}

void SHA1::Reset(void)
{
	state[0]=0x67452301U;
	state[1]=0xEFCDAB89U;
	state[2]=0x98BADCFEU;
	state[3]=0x10325476U;
	state[4]=0xC3D2E1F0U;

	count[0]=count[1]=0;
	}

void SHA1::Update(const void * const ptr,const uint len)
{
	const uchar * const data=static_cast<const uchar *>(ptr);

		// count[] stores number of bits, not bytes

	uint j=(count[0] >> 3) & 63;

	{ const uint len_bits_low=(len << 3) & 0xffffffffU;

	count[0]=(count[0] + len_bits_low) & 0xffffffffU;
	if (count[0] < len_bits_low)
		count[1]++;

	count[1]+=(len >> 29); }

	if ((j + len) > 63) {
		uint i=64-j;
		memcpy(&buffer[j],data,i);
		transform(buffer);
		for (;i + 63 < len;i+=64)
			transform(&data[i]);
		memcpy(buffer,&data[i],len - i);
		}
	  else
		memcpy(&buffer[j],data,len);
	}

void SHA1::GetBinary(uchar digest[20])
{
	uchar finalcount[8];

	{ for (uint i=0;i < lenof(finalcount);i++)
		finalcount[i]=(uchar)((
				count[(i >= 4 ? 0 : 1)] >> ((3-(i & 3)) * 8)
								) & 0xff); }

	Update((const uchar *)"\200",1);
	while ((count[0] & 504) != 448)
		Update((const uchar *)"\0",1);

	Update(finalcount,8);  		// should cause a transform()

	{ for (uint i=0;i < BINARY_DIGEST_SIZE;i++)
		digest[i]=(uchar)((state[i>>2] >> ((3-(i & 3)) * 8) ) & 0xff); }

		// wipe variables

	zero(buffer);
	zero(state);
	zero(count);
	zero(finalcount);
	}

void SHA1::GetAscii(char *digest)
{
	uchar d[BINARY_DIGEST_SIZE];
	GetBinary(d);

	const char * const hex="0123456789abcdef";
	for (uint i=0;i < lenof(d);i++) {
		*(digest++)=hex[d[i]>>4];
		*(digest++)=hex[d[i]&15];
		}

	*digest='\0';
	}

#ifdef SHA1_TEST_PROGRAM
#include <stdio.h>
int main(char **,sint)
{
	{ SHA1 hash("abc",3);
	char digest[hash.BINARY_DIGEST_SIZE*2+1];
	hash.GetAscii(digest);
	if (!strcmp(digest,"a9993e364706816aba3e25717850c26c9cd0d89d"))
		printf("test1 OK\n");
	  else {
		printf("test1 FAILED (%s)\n",digest);
		return 1; }}

	{ SHA1 hash;
	char digest[hash.BINARY_DIGEST_SIZE*2+1];
	static uchar buf[1000];
	memset(buf,'a',sizeof(buf));
	for (uint i=0;i < 1000;i++)
		hash.Update(buf,sizeof(buf));
	hash.GetAscii(digest);
	if (!strcmp(digest,"34aa973cd4c4daa4f61eeb2bdbad27316534016f"))
		printf("test2 OK\n");
	  else {
		printf("test2 FAILED (%s)\n",digest);
		return 1; }}

	return 0;
	}
#endif
