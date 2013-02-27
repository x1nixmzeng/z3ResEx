/*
	z3ResEx
	Written by x1nixmzeng

	z3MSF.h
		Updated Feb 2013
*/

#ifndef Z3MSF_H
#define Z3MSF_H

// ** METHOD 1 **
//	COMPRESSED uses an xor-based scramble (see z3Xor.h) with RLE compression (see z3Rle.h)
//	COMPRESSED2 indicates the file is encrypted with the same fileindex key
//	UNCOMPRESSED files are either video or large audio banks
//

enum FILEINDEX_ENTRY_METHOD
{
	FILEINDEX_ENTRY_COMPRESSED  = 0x0,
	FILEINDEX_ENTRY_COMPRESSED2,
	FILEINDEX_ENTRY_UNCOMPRESSED
};

// ** METHOD 2 **
//	COMPRESSED (see above)
//	UNCOMPRESSED (see above )
//

enum FILEINDEX_ENTRY_METHOD2
{
	FILEINDEXITEM_COMPRESSED    = 0xFC,
	FILEINDEXITEM_UNCOMPRESSED  = 0xFD
};

//
//	fileindex.msf structure
//	20-bytes (+ filename)
//

struct FILEINDEX_ENTRY
{
	unsigned int size;          // Uncompressed filesize
	unsigned int offset;        // MRF file position
	unsigned int zsize;         // Size of compressed data
	unsigned int xorkey;        // Secret key used for file scrambling
	unsigned short lenMRFN;     // MRF container name length
	unsigned short lenName;     // Filename length
};

//
//	fileindex.msf structure - method 2
//	21-bytes (+ filename)
//

#pragma pack( push )
#pragma pack( 1 )
struct FILEINDEX_ENTRY2
{
	unsigned int size;          // Uncompressed filesize
	unsigned int offset;        // MRF file position
	unsigned int zsize;         // Size of compressed data
	unsigned int xorkey;        // Secret key used for file scrambling
	unsigned short mrfIndex;    // MRF file index
	unsigned char type;         // Storage method (FILEINDEX_ENTRY_METHOD2 enum)
	//unsigned short lenName;     // Filename length
};
#pragma pack( pop )

struct FILEINDEX_HEADER
{
	unsigned int size;
	unsigned int unknown1;
	unsigned short unknown2;
	unsigned short mrfIndexLen;
};

#endif
