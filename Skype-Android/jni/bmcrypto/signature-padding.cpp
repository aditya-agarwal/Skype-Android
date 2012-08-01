#include <string.h>
#include "sha1.hpp"
#include "signature-padding.hpp"

// Implements ISO 9796-2 padding for RSA signatures.

uint signature_padding::encode(const uchar * const src,const uint src_len,
						const uint block_len,uchar * const dest_block)
{
		// Fills dest_block[] and returns message remainder length (which
		//   could be 0). That many trailing bytes of src[] must be
		//   transmitted separately from dest_block[].
		// Guarantees that dest_block[0] < 0x80
		// Caller must allocate at least block_len bytes for dest_block[].
		// All lengths are in bytes. block_len must be >= 32.
		// This function is thread-safe.

	if (block_len < 32)
		return 0;

	dest_block[block_len-1]=0xbc;

	SHA1 hash(src,src_len);
	hash.GetBinary(&dest_block[block_len-1-hash.BINARY_DIGEST_SIZE]);

	const uint max_embedded_msg_len=block_len - (1+hash.BINARY_DIGEST_SIZE+1);
	if (src_len <= max_embedded_msg_len) {
		dest_block[0]=0x4b;
		const uint b_len=max_embedded_msg_len - src_len;
		for (uint i=0;i < b_len;i++)
			dest_block[1+i]=0xbb;
		dest_block[b_len]&=0xfe;
		memcpy(&dest_block[1+b_len],src,src_len);
		return 0;
		}
	  else {
		dest_block[0]=0x6a;
		memcpy(&dest_block[1],src,max_embedded_msg_len);
		return src_len - max_embedded_msg_len;
		}
	}

uint signature_padding::decode(
				const uchar * const src_block,const uint block_len,
				const uchar * const src_remainder,const uint remainder_len,
				uchar * const dest,uint &dest_len)
{
		// Returns 0 if the padding was invalid; otherwise returns nonzero,
		//   fills dest[] with the portion of message that was transmitted
		//   within src_block[], and sets dest_len accordingly.
		// Caller must allocate at least block_len bytes for dest[].
		// All lengths are in bytes. block_len must be >= 32.
		// This function is thread-safe.

	if (block_len < 32)
		return 0;

	if (src_block[block_len-1] != 0xbc)
		return 0;

	SHA1 hash;
	const uint hash_start_pos=block_len-1-hash.BINARY_DIGEST_SIZE;

	if (src_block[0] == 0x6a && remainder_len) {

			// partial message recovery

		dest_len=hash_start_pos-1;
		memcpy(dest,src_block+1,dest_len);
		}
	  else {

			// total message recovery

		if (remainder_len || (src_block[0]&0xf0) != 0x40)
			return 0;

		for (uint i=0;;i++) {
			if (i >= hash_start_pos)
				return 0;
			const uint nibble=src_block[i] & 0x0f;
			if (nibble == 0x0b)
				continue;
			if (nibble != 0x0a)
				return 0;

			dest_len=hash_start_pos - (i+1);
			memcpy(dest,&src_block[i+1],dest_len);
			break;
			}
		}

	hash.Update(dest,dest_len);
	hash.Update(src_remainder,remainder_len);

	uchar digest[hash.BINARY_DIGEST_SIZE];
	hash.GetBinary(digest);
	return !memcmp(digest,&src_block[hash_start_pos],hash.BINARY_DIGEST_SIZE);
	}

#ifdef SIGNATURE_PADDING_TEST_PROGRAM
#include <stdio.h>
#include <sha1.cpp>
int main(char **,sint)
{
	uchar block[256];
	uchar buf[1000];

	const char *failure_reason=NULL;
	uint len;
	for (len=0;len < lenof(buf);len++) {

		{for (uint i=0;i < len;i++)
			buf[i]=(uchar)((len+i) & 0xff);}
		const uint remainder_len=
					signature_padding::encode(buf,len,sizeof(block),block);
		if (remainder_len > len) {
			failure_reason="invalid remainder_len";
			break;
			}

		uchar dest_buf[lenof(buf)];
		{for (uint i=0;i < len;i++)
			dest_buf[i]=(uchar)(buf[i] ^ 0x55);}

		uint embedded_len;
		if (!signature_padding::decode(	block,sizeof(block),
										&buf[len-remainder_len],remainder_len,
										dest_buf,embedded_len)) {
			failure_reason="decode() returned 0";
			break;
			}

		if (embedded_len+remainder_len != len) {
			failure_reason="invalid embedded_len";
			break;
			}
		if (memcmp(dest_buf,buf,embedded_len)) {
			failure_reason="invalid embedded part of message";
			break;
			}
		}

	if (failure_reason != NULL) {
		printf("Test failed with message length %u: %s\n",len,failure_reason);
		return 1;
		}
	  else {
		printf("All tests OK\n");
		return 0;
		}
	}
#endif
