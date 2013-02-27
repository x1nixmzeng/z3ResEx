/*
	z3ResEx
	Written by x1nixmzeng

	z3Xor.h
		From the original Z3Ex project
*/

#ifndef Z3XOR_H
#define Z3XOR_H

#include <windows.h>

namespace z3Xor
{
	void rs3Unscramble
	(
		unsigned char *srcBuffer,	// Source buffer data
		DWORD srcSize,				// Source buffer size
		DWORD xorkey				// Initial XOR value
	);
};

#endif
