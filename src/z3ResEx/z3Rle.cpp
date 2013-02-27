#include "z3Rle.h"

namespace z3Rle
{
	#include "z3RleTable.inl"

	bool decodeSize
	(
		unsigned char *data,
		unsigned int &value,
		unsigned int &length
	)
	{
		codedSizeByte sizeByte;
		unsigned int bitSHL;

		value = 0;	// Reset returned size value
		length = 0;	// Length (in bytes) of size data

		bitSHL = 0;	// Reset number of bits to shift left

		do
		{
			// Check shift value (indicates invalid starting position)
			if( bitSHL > 32 ) return false;

			sizeByte = *(codedSizeByte *)data;
			value |= ( static_cast<unsigned int>( sizeByte.data ) << bitSHL );

			bitSHL += 7;	// Bits of data per 'codedSizeByte'
			++data;			// Move along buffer
		}
		while( sizeByte.notLastData );

		// Calculate the actual length of data
		length = bitSHL /= 7;

		return true;
	}

	bool decodeInstruction
	(
		unsigned char *data,
		unsigned int &count,
		unsigned char *dataLength,
		unsigned char *buff,
		unsigned int &bufCount
	)
	{
		unsigned char cmdMarker;
		unsigned int instruction, instructionExSize, buf32;

		count = 0;

		// Read the command marker from the buffer
		cmdMarker = *(unsigned char *)(data+count);
		++count;
	
		// Lookup the instruction
		instruction = z3RleInstructions[ cmdMarker ];

		// Length of additional bytes (5-bits of instruction)
		instructionExSize = instruction >> 11;

		// 5 sizes are supported in the client (0,1,2,3,4)
		if( instructionExSize > 4 )
			return false;

		buf32 = 0;

		// Read these additional bytes
		for( unsigned int i(0 ); i < instructionExSize; ++i, ++count )
		{
			// Read another byte into buf32
			unsigned int tmp = data[ count ];
			tmp <<= ( i * 8 );
			buf32 |= tmp;
		}

		// Check command marker for method
		if( cmdMarker & 3 )
		{
			// Using the destination buffer as the source (copying existing data)

			unsigned int srcOffset, msgLength;

			// Data can be up to 2,047 bytes from current position
			srcOffset = ( instruction & 0x700 ) + buf32;
			msgLength = ( instruction & 0xFF );

			// Check for invalid modulus and invalid source offset (corrupted data)
			if( ( srcOffset == 0 ) || ( srcOffset > bufCount ) )
				return false;

			// Copy data from existing buffer
			for(unsigned int i = 0; i < msgLength; ++i )
				*(buff+bufCount+i) = *(buff+bufCount-(srcOffset - (i % srcOffset)));

			bufCount += msgLength;
		}
		else
		{
			// Using the source buffer as the source (inserting new data)

			unsigned int msgLength;

			msgLength = ( instruction & 0xFF ) + buf32;

			// Check data source (source buffer) has enough data
			if( data+count+msgLength > dataLength )
				return false;

			// Copy data from buffer
			memcpy( buff+bufCount, data+count, msgLength );
			count += msgLength;
		
			bufCount += msgLength;
		}

		return true;
	}
};