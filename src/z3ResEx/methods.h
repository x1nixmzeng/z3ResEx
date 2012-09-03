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
bool fsRle( TMemoryStream &src, TMemoryStream &dst /*todo: expected size here?*/ )
{
	unsigned int expectedSize, len;
	unsigned char *pData( src.Data() +4 );

	if( !( z3Rle::decodeSize( pData, expectedSize, len ) ) )
	{
		dst.Close();
		printf("ERROR: Problems decoding RLE buffer size\n");
		return false;
	}

	/*todo: check expected size and error if do not match*/

	pData += len;

	unsigned char *tmpBuffer( new unsigned char[ expectedSize ] );
	unsigned int tmpOffset( 0 );

	while( tmpOffset < expectedSize )
	{
		if( !( z3Rle::decodeInstruction( pData, len, tmpBuffer, tmpOffset ) ) )
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

// todo

#endif
