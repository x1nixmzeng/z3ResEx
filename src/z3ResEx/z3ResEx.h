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

// TStream class
#include "mbuffer.h"
#include "fbuffer.h"

// Fileindex structures
#include "z3MSF.h"

// CryptoLib project headers
#include "filters.h"
#include "osrng.h"
#include "eccrypto.h"

// Decryption methods
#include "z3Rle.h"
#include "z3Xor.h"

// Parameter handling
#include "targs.h"

using namespace CryptoPP;
using std::string;
using std::vector;

#if defined( _DEBUG )
#define VERBOSE_BY_DEFAULT
#endif

#define z3ResExMsgLen		1024
#define MAX_ERRORS			50

class z3ResEx
{
	static char		msfName[];
	
	char			m_lastMsg[ z3ResExMsgLen ];
	
	unsigned char	*m_fileindexKey;
	unsigned int	m_fileindexKeyLength;
	
	int				m_fileindexVer;
	
	bool			m_doExtraction;
	bool			m_listContents;
	bool			m_verboseMessages;

	// Setting errors
	void setMessage( const char * );
	void setMessage( const char *, const char * );
	void setMessage( const char *, const unsigned int );

	// Misc functions
	std::string fsRename( const char *, const char * ) const;
	void unpackStringEx( TMemoryStream &, unsigned char *&, const unsigned int ) const;
	void fsCreatePath( std::string &strPath ) const;

	// Extraction functions
	bool extractItem2( FILEINDEX_ENTRY2 &, const string &, char * );
	bool extractItem( FILEINDEX_ENTRY &, unsigned char, char *, char * );

	// Helper functions
	bool z3Decrypt( TMemoryStream &, TMemoryStream &, unsigned char *, unsigned int );
	bool fsRle( TMemoryStream &, TMemoryStream &, bool isMSF = false );
	void fsXor( TMemoryStream &, unsigned int ) const;

	// Fileindex loading function
	bool fsReadMSF( TMemoryStream &, unsigned char *, unsigned int, int );

	// Fileindex parsing functions
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
