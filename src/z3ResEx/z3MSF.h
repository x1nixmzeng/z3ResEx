/*
	z3ResEx
	Written by x1nixmzeng

	z3MSF.h
		Contains parts of the Z3Ex project
*/

#ifndef _H_Z3MSF_
#define _H_Z3MSF_

#include <windows.h>

const char *msfName( "fileindex.msf" );

enum FILEINDEX_ENTRY_METHOD
{
	FILEINDEX_ENTRY_COMPRESSED		= 0x0,
	FILEINDEX_ENTRY_COMPRESSED2,
	FILEINDEX_ENTRY_UNCOMPRESSED
};

//
//	fileindex.msf structure
//	20-bytes
//	
//	NOTE: FILEINDEX_ENTRY records follows a 1-byte compression flag
//
//		3 known types (RaiderZ only uses type 0)
//		
//		0	Compressed		Data should be fed through XorData() before decompressing
//		1	Compressed++	Data needs unscrambling, which converts it to type 0
//		2	Uncompressed	(GUNZ2 FSB) Raw data - BUT the values have been scrambled (TODO)
//
struct FILEINDEX_ENTRY
{
	DWORD size;		// Uncompressed filesize
	DWORD offset;	// MRF file position
	DWORD zsize;	// Size of compressed data
	DWORD xorkey;	// Secret key used for file scrambling
	short lenMRFN;	// MRF container name length
	short lenName;	// Filename length
};

#endif
