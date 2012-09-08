/*
	z3ResEx
	Written by x1nixmzeng

	methods.h
		Wrappers for decryption and data manipulation
*/

#ifndef _H_METHODS_
#define _H_METHODS_

// Included from the CryptoLib project
#include "filters.h"
#include "osrng.h"
#include "eccrypto.h"

// Included local
#include "keys.h"
#include "mbuffer.h"
#include "z3MSF.h"

#include "z3Rle.h"
#include "z3Xor.h"

using namespace CryptoPP;

/*
	Crypto++
*/
bool z3Decrypt( unsigned char* key, TMemoryStream &src, TMemoryStream &dst )
{
	StringSource keyStr( key, Z3_KEY_LENGTH, true );
	
	AutoSeededRandomPool rng;
	ECIES<ECP>::Decryptor ellipticalEnc( keyStr );
	
	unsigned char *tmpBuffer( new unsigned char[ src.Size() ] );

	DecodingResult dr = ellipticalEnc.Decrypt( rng, src.Data(), src.Size(), tmpBuffer );
	
	if( !( dr.isValidCoding ) || ( dr.messageLength == 0 ) )
	{
		delete tmpBuffer;
		return false;
	}

	dst.Write( tmpBuffer, dr.messageLength );

	delete tmpBuffer;
	
	return true;
}

/*
	z3Rle
*/
bool fsRle( TMemoryStream &src, TMemoryStream &dst, bool isMSF = false )
{
	unsigned int msfSizeFlag;
	unsigned int expectedSize, len;
	unsigned char *pData( src.Data() ), *pDataEnd( pData + src.Size() );

	if( isMSF )
	{
		// Read the expected size from data
		msfSizeFlag = src.ReadUInt();
		pData += 4;
	}

	if( !( z3Rle::decodeSize( pData, expectedSize, len ) ) )
	{
		dst.Close();
		printf("ERROR: Problems decoding RLE buffer size\n");
		return false;
	}

	if( isMSF && !( msfSizeFlag == expectedSize ) )
	{
		dst.Close();
		printf("ERROR: Unexpected MSF buffer size\n");
		return false;
	}

	// Skip the length of the expected size
	pData += len;

	unsigned char *tmpBuffer( new unsigned char[ expectedSize ] );
	unsigned int tmpOffset( 0 );

	while( tmpOffset < expectedSize )
	{
		if( !( z3Rle::decodeInstruction( pData, len, pDataEnd, tmpBuffer, tmpOffset ) ) )
		{
			delete tmpBuffer;
			printf("ERROR: Problems decoding RLE buffer\n");

			return false;
		}

		pData += len;
	}

	dst.Write( tmpBuffer, expectedSize );

	delete tmpBuffer;

	return true;
}

/*
	z3Xor
*/
__inline void fsXor( FILEINDEX_ENTRY &info, TMemoryStream &src )
{
	z3Xor::rs3Unscramble( src.Data(), src.Size(), info.xorkey );
}

/*
	Misc
*/
bool fsReadMSF( TMemoryStream &msf, unsigned char *z3Key )
{
	TMemoryStream fileIndex, fileIndex_dec;

	// Check we can open the file for reading (NOTE: this is already check in main)
	if( !( fileIndex.LoadFromFile( msfName ) ) )
	{
		return false;
	}

	// Double-check the filesize (NOTE: this is already check in main)
	if( !( fileIndex.Size() > 0 ) )
	{
		fileIndex.Close();
		return false;
	}

	// Attempt to decrypt the data
	if( !( z3Decrypt( z3Key, fileIndex, fileIndex_dec ) ) )
	{
		fileIndex.Close();
		return false;
	}

	fileIndex.Close();
	
	// Attempt to uncompress the data
	if( !( fsRle( fileIndex_dec, msf, true ) ) )
	{
		fileIndex_dec.Close();
		return false;
	}

	// Success! File has been converted to plaintext!
	
	fileIndex.Close();
	fileIndex_dec.Close();

	return true;
}

#endif
