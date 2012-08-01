#ifndef _SIGNATURE_PADDING_HPP
#define _SIGNATURE_PADDING_HPP

#include "def.h"

class signature_padding {
	public:

	static uint encode(const uchar * const src,const uint src_len,
						const uint block_len,uchar * const dest_block);
		// Fills dest_block[] and returns message remainder length (which
		//   could be 0). That many trailing bytes of src[] must be
		//   transmitted separately from dest_block[].
		// Guarantees that dest_block[0] < 0x80
		// Caller must allocate at least block_len bytes for dest_block[].
		// All lengths are in bytes. block_len must be >= 32.
		// This function is thread-safe.

	static uint decode(const uchar * const src_block,const uint block_len,
				const uchar * const src_remainder,const uint remainder_len,
				uchar * const dest,uint &dest_len);
		// Returns 0 if the padding was invalid; otherwise returns nonzero,
		//   fills dest[] with the portion of message that was transmitted
		//   within src_block[], and sets dest_len accordingly.
		// Caller must allocate at least block_len bytes for dest[].
		// All lengths are in bytes. block_len must be >= 32.
		// This function is thread-safe.
	};

#endif
