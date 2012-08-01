#ifndef __tc_crypto_hpp__
#define __tc_crypto_hpp__

#include <stdlib.h>
#include <stdio.h>
#include "arc4.hpp"
#include "bignum-funcs.hpp"
#include "sha1.hpp"
#include "trsa.hpp"
#include "types.hpp"
#include "AttributeContainer.hpp"
#include "signature-padding.hpp"
#include "crc.hpp"

// key sizes in bits
// root key should not be set below 1024
// server key size has to be at least 336 bits, otherwise the shared secret cannot be encrypted
//
#define SERVERKEYSIZE 384
#define ROOTKEYSIZE 1024

// these idendify key components in key containers
//
enum KEYATTRIBUTES {
	ATTR_ROOT_PRIVATE_KEY=0,
	ATTR_ROOT_PUBLIC_KEY,
	ATTR_SERVER_PRIVATE_KEY,
	ATTR_SERVER_PUBLIC_KEY,
	ATTR_SERVER_SIGNED_PUBLIC_KEY
};

// random number feed must be externally supplied
//
extern uchar tc_random_func(void *s);

// see tc_crypto.cpp for function descriptions
//
class tc_crypto
{
public:
	static uchar * GetContainerBlob(AttributeContainer& c,uint key,uint *lptr,char *format=NULL,uint len=0);
	static void GenerateRootKey(AttributeContainer& dest);
	static bool GenerateAndSignServerKey(AttributeContainer& dest,AttributeContainer& SigningKey);
	static bool VerifyServerKey(AttributeContainer& src,AttributeContainer& VerificationKey);
	static bool EncryptSecret(const uchar data[20],uchar encrypted_data[SERVERKEYSIZE/8+2],AttributeContainer& EncryptionKey);
	static bool DecryptSecret(const uchar encrypted_data[SERVERKEYSIZE/8+2],uchar data[20],AttributeContainer& DecryptionKey);

	static bool LoadKeys(AttributeContainer& container,char* filename,char* passphrase);
	static bool SaveKeys(AttributeContainer& container,char *filename,char* passphrase);
};

#endif
