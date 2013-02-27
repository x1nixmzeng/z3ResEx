/*
	z3ResEx
	Written by x1nixmzeng

	z3Rle.h
		Contains parts of the z3ResourceExtractor-wip project
		Research work inspired by skyflox
*/

#ifndef Z3RLE_H
#define Z3RLE_H

#include <Windows.h>

namespace z3Rle
{
	struct codedSizeByte
	{
		/*
			Method to store variable size value
			Keeps reading until the 8th bit is 0

			NOTE: Only a basic shift overflow check

			This method looks common in lz compression (lz.c uses it)
		*/

		unsigned char data	: 7;
		bool notLastData	: 1;
	};

	bool decodeSize
	(
		unsigned char *data,		// Source buffer
		unsigned int &value,		// Destination size value
		unsigned int &length		// Destination length of encoded size value
	);

	bool decodeInstruction
	(
		unsigned char *data,		// Source buffer
		unsigned int &count,		// Size of last instruction
		unsigned char *dataLength,	// End of source buffer
		unsigned char *buff,		// Destination buffer
		unsigned int &bufCount		// Destination buffer offset
	);
};

#endif
