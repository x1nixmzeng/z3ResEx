#include "z3Xor.h"

namespace z3Xor
{
	void rs3Unscramble
	(
		unsigned char *srcBuffer,
		DWORD srcSize,
		DWORD xorkey
	)
	{
		// Unscramble the 32-bit blocks

		DWORD dBlocks = srcSize >> 2;
		while( dBlocks )
		{
			DWORD tmp;
			tmp = *(DWORD *)srcBuffer; // read 32-bit value
		
			xorkey += tmp;
			tmp ^= xorkey;
		
			*(DWORD *)srcBuffer = tmp; // write 32-bit value
			srcBuffer += 4;
			--dBlocks;
		}
	
		// Unscramble the remaining data (1-3 bytes)

		BYTE bAlign = (BYTE)(srcSize & 3);
		if( bAlign )
		{
			DWORD lastBlock;
			int i;

			lastBlock = 0;
			i = 0;

			// read remaining 8/16/24-bit value
			while( i < bAlign )
			{
				DWORD tmp;
				tmp = *(BYTE *)(srcBuffer + i);
				tmp <<= i << 3;
				lastBlock |= tmp;

				++i;
			}

			xorkey += lastBlock;
			lastBlock ^= xorkey;
		
			// write remaining 8/16/24-bit value
			i = 0;
			while( i < bAlign )
			{
				*(BYTE *)(srcBuffer + i) = (BYTE)lastBlock;
				lastBlock >>= 8;

				++i;
			}
		}
	}
};
