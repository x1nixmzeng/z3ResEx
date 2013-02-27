/*
	z3ResEx
	Written by x1nixmzeng

	z3ResEx
	Core application logic
*/
#ifndef Z3RESEX_H
#define Z3RESEX_H

#include <stdio.h>
#include <string>
#include <vector>

// Import TStream class
#include "mbuffer.h"
#include "fbuffer.h"

// Import fileindex definitions and encryption keys
#include "z3MSF.h"

// Import data manipulation methods

// Included from the CryptoLib project
#include "filters.h"
#include "osrng.h"
#include "eccrypto.h"

#include "z3Rle.h"
#include "z3Xor.h"

using namespace CryptoPP;
using std::string;
using std::vector;

// Import argument handler
#include "targs.h"

#if defined( _DEBUG )
#define VERBOSE_BY_DEFAULT
#endif

#define z3ResExMsgLen		1024

class z3ResEx
{
	static char msfName[];

	char m_lastMsg[1024];

	unsigned char *m_fileindexKey;
	unsigned int m_fileindexKeyLength;

	int m_fileindexVer;

	bool m_doExtraction;
	bool m_listContents;
	bool m_verboseMessages;

	std::string fsRename( char *, char * ) const;
	void unpackStringEx( TMemoryStream &, char *&, const unsigned int ) const;
	void fsCreatePath( std::string &strPath ) const;

	bool extractItem( FILEINDEX_ENTRY &, unsigned char, char *, char * );

	bool z3Decrypt( TMemoryStream &, TMemoryStream &, unsigned char *, unsigned int );
	bool fsRle( TMemoryStream &, TMemoryStream &, bool isMSF = false );
	void fsXor( FILEINDEX_ENTRY &, TMemoryStream & );

	bool fsReadMSF( TMemoryStream &, unsigned char *, unsigned int, int );

	void parseMsf( TMemoryStream & );
	void parseMsfMethod2( TMemoryStream & );
	
public:
	// Constructor
	z3ResEx( );
	
	// Modifiers
	bool setFlags( const targs & );
	void Run( );

	// Accessors
	const char *lastMessage( ) const;
	void PrintUsage( ) const;
};

#endif
