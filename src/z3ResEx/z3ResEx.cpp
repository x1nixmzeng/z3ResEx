#include "z3ResEx.h"
#include "keys.h"

char z3ResEx::msfName[] = "fileindex.msf";

z3ResEx::z3ResEx( )
	: m_fileindexKey( nullptr )
	, m_fileindexKeyLength( 0 )
	, m_fileindexVer( 0 )
	, m_doExtraction( true )
	, m_listContents( false )
#if defined( VERBOSE_BY_DEFAULT )
	, m_verboseMessages( true )
#else
	, m_verboseMessages( false )
#endif
{
	*m_lastMsg = 0;
}

void z3ResEx::PrintUsage( ) const
{
	puts("Usage: z3ResEx.exe DIR -v|-x|-l");
	{
		puts(" <DIR>                  * Initial directory");
		puts(" -v (--verbose)         * Verbose log messages");
		puts(" -x (--no-extraction)   * Do not extract files");
		puts(" -l (--list-filesystem) * Show list of filenames (overrides -x)");
		puts("                       (* optional argument)");
	}
	puts("");
	{
		puts("z3ResEx.exe             Extract from current directory)");
		puts("z3ResEx.exe C:\\RaiderZ  Extract from C:\\RaiderZ");
		puts("z3ResEx.exe . -l        List filesystem in current directory");
	}
	puts("");
}

bool z3ResEx::setFlags( const targs &args )
{
	if( args.count() > 1 )
	{
		if( SetCurrentDirectoryA( args.getArgCStr(1) ) == 0 )
		{
			sprintf(m_lastMsg, "ERROR: Cannot set current path to \"%s\"\n", args.getArgCStr(1) );
			return false;
		}

		if( args.hasArg("--usage") )
		{
			PrintUsage();

			// Also stop execution here (but successfully)
			return true;
		}

		// Allow verbose toggle when not true by default
		if( !( m_verboseMessages ) )
			m_verboseMessages = ( args.hasArg("-v") || args.hasArg("--verbose") );

		// Disable file extraction
		if( args.hasArg("-x") || args.hasArg("--no-extraction") )
			m_doExtraction = false;

		// List the filesystem contents (no extraction)
		if( args.hasArg("-l") || args.hasArg("--list-filesystem") )
		{
			m_listContents = true;
			m_doExtraction = false;
		}
	}

	// Print current flags
	{
		if( m_doExtraction )
			puts("[Enabled] Extracting");

		if( m_verboseMessages )
			puts("[Enabled] Verbose messages");

		if( m_listContents )
			puts("[Enabled] List filesystem");
	}

	return true;
}

const char *z3ResEx::lastMessage( ) const
{
	return m_lastMsg;
}

std::string z3ResEx::fsRename( char *strMrf, char *strName ) const
{
	std::string name( "datadump/" );

	// Append the MRF name
	name += strMrf;
	// Now remove the MRF extension (.mrf, .001, .002, etc)
	name = name.substr( 0, name.rfind('.') );
	// Append the filename
	name += "/";
	name += strName;

	return name;
}

void z3ResEx::unpackStringEx( TMemoryStream &msf, char *&buf, const unsigned int len ) const
{
	buf = new char[ len +1 ];
	msf.Read( buf, len );
	buf[len] = 0;

	/*
		Simple xor added to strings in later clients
			buf[0] is the xor character
			buf[1] to buf[size-1] contains the xored string
	*/
	if( m_fileindexVer == 1 )
	{
		const char encKey( buf[0] );	// First byte is the key
		unsigned int i = 1;				// Second byte starts the data

		while( i < len )
		{
			// This unscrambles the string into the same buffer
			buf[i-1] = buf[i] ^ encKey;
			++i;
		}

		buf[len-1] = 0;

		// Buffer now has 2 null characters at the end
	}
}

void z3ResEx::Run( )
{
	// Check the fileindex exists
	if( TFileSize( msfName ) == 0 )
	{
		sprintf(m_lastMsg, "ERROR: Unable to open file (%s)\n", msfName);
	}
	else
	{
		TMemoryStream msf;

		m_fileindexKey			= nullptr;
		m_fileindexKeyLength	= 0;

		// Brute-force the key (fileindex method 1)
		unsigned int keyIndex( 0 );

		while( ( keyIndex < Z3_KEY_LIST_LENGTH ) && ( msf.Size() == 0 ) )
		{
			if( fsReadMSF( msf, Z3_KEY_LIST[ keyIndex ], Z3_KEY_LENGTH, 0 ) )
			{
				m_fileindexKey			= Z3_KEY_LIST[ keyIndex ];
				m_fileindexKeyLength	= Z3_KEY_LENGTH;
				m_fileindexVer			= 0;
			}

			++keyIndex;
		}

		// Continue to brute-force the key (second method)
		if( m_fileindexKey == nullptr )
		{
			if( fsReadMSF( msf, Z3_KEY_GUNZ2_METHOD2, Z3_KEY_LENGTH_METHOD2, 1 ) )
			{
				m_fileindexKey			= Z3_KEY_GUNZ2_METHOD2;
				m_fileindexKeyLength	= Z3_KEY_LENGTH_METHOD2;
				m_fileindexVer			= 1;
			}
		}

		if( !( ( m_fileindexKey == nullptr ) && ( msf.Size() == 0 ) ) )
		{
			// The key has been found
			msf.Seek( 0, bufo_start );
			parseMsf( msf );
		}
		else
		{
			// No key found or incompatiable file (not checked)
			sprintf(m_lastMsg, \
				"ERROR: This file is using an updated key or unsupported method\n");
		}

		msf.Close();
	}
}

bool z3ResEx::fsReadMSF
(
	TMemoryStream &msf,
	unsigned char *key,
	unsigned int keylen,
	int ver
)
{
	TMemoryStream fileIndex, fileIndex_dec;

	// Check we can open the file for reading
	if( !( fileIndex.LoadFromFile( msfName ) ) )
	{
		return false;
	}

	// Double-check the filesize
	if( !( fileIndex.Size() > 0 ) )
	{
		fileIndex.Close();
		return false;
	}

	switch( ver )
	{
		case 0 :
		{
			// Attempt to decrypt the data
			if( !( z3Decrypt( fileIndex, fileIndex_dec, key, keylen ) ) )
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

			break;
		}

		case 1 :
		{
			// NOTE: 3 unknown bytes read at start of buffer

			fileIndex_dec.LoadFromBuffer( (void *)(fileIndex.Data() + 3), fileIndex.Size() -3 );
			fileIndex.Close();

			// Attempt to decrypt the data
			if( !( z3Decrypt( fileIndex_dec, msf, key, keylen ) ) )
			{
				fileIndex_dec.Close();
				return false;
			}

			break;
		}
	}

	fileIndex.Close();
	fileIndex_dec.Close();

	return true;
}

bool z3ResEx::z3Decrypt
(
	TMemoryStream &src,
	TMemoryStream &dst,
	unsigned char *key,
	unsigned int keylen
)
{
	StringSource keyStr( key, keylen, true );
	
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

bool z3ResEx::fsRle( TMemoryStream &src, TMemoryStream &dst, bool isMSF )
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

void z3ResEx::fsXor( FILEINDEX_ENTRY &info, TMemoryStream &src )
{
	z3Xor::rs3Unscramble( src.Data(), src.Size(), info.xorkey );
}

void z3ResEx::parseMsfMethod2( TMemoryStream &msf )
{
	unsigned short strLen( 0 );
	char *strBuffer( nullptr );

	FILEINDEX_HEADER fiHead;

	// Now includes a 12-byte header (largely unknown)
	msf.Read( &fiHead, sizeof( FILEINDEX_HEADER ) );

	// List of filenames (NOTE: should be paired with FILE)
	//std::pair<string, FILE * >
	vector<string > vecMsfNames;
	vecMsfNames.resize( fiHead.mrfIndexLen );
			
	// MRF filenames are now packed in a list
	for( unsigned short i( 0 ); i != fiHead.mrfIndexLen; ++i )
	{
		strLen = msf.ReadUShort();
		unpackStringEx( msf, strBuffer, strLen );

		vecMsfNames[i] = strBuffer;

		delete strBuffer;
	}

	// Files are now listed (similar to before)
	FILEINDEX_ENTRY2 fiItem;

	while( msf.Position() < msf.Size() )
	{
		msf.Read( &fiItem, sizeof( FILEINDEX_ENTRY2 ) );

		strLen = msf.ReadUShort();
		unpackStringEx( msf, strBuffer, strLen );
		
		// TODO: Extract file data

		printf( "Found %s\n", strBuffer );

		delete strBuffer;
	}
}

void z3ResEx::fsCreatePath( std::string &strPath ) const
{
	int pathLoc( strPath.find('/') );

	while( !( pathLoc == std::string::npos ) )
	{
		CreateDirectoryA( strPath.substr( 0, pathLoc ).c_str(), nullptr );
		pathLoc = strPath.find( '/', pathLoc+1 );
	}
}

bool z3ResEx::extractItem( FILEINDEX_ENTRY &info, unsigned char method, char *strMrf, char *strName )
{
	TFileStream mrf( strMrf );

	if( !( mrf.isOpen() ) )
	{
		sprintf(m_lastMsg, "ERROR: Unable to open file (%s)\n", strMrf );
		return false;
	}

	// Format the output filename
	std::string fname( fsRename( strMrf, strName ) );
	
	// UNFORCED EXTRACTION
	// If file already exists, ignore it
	if( TFileSize( fname.c_str() ) == info.size )
	{
		mrf.Close();
		return true;
	}

	unsigned char *buf( new unsigned char[ info.zsize ] );

	// Load MRF data into buffer
	mrf.Seek( info.offset, bufo_start );
	mrf.Read( buf, info.zsize );
	mrf.Close();

	// Copy into TStream
	TMemoryStream fdata;
	fdata.LoadFromBuffer( buf, info.zsize );
	delete buf;

	printf
	(	
		( m_doExtraction ? "Saving %s.. " : "Checking %s.. " ),
		fname.substr( fname.rfind('/') +1 ).c_str()
	);

	// Create path only when extraction is flagged
	if( m_doExtraction )
		fsCreatePath( fname );

	switch( method )
	{
		// Compressed, most files
		case FILEINDEX_ENTRY_COMPRESSED :
		{
			fsXor( info, fdata );

			TMemoryStream fdata_raw;
			if( fsRle( fdata, fdata_raw ) )
			{
				if( m_doExtraction )
					fdata_raw.SaveToFile( fname.c_str() );

				puts(" ..done!");
			}
		
			// fsRle will display any errors

			fdata_raw.Close();
			break;
		}

		// Encrypted and compressed, some system data (GunZ 2)
		case FILEINDEX_ENTRY_COMPRESSED2 :
		{
			TMemoryStream fdata_dec;
			z3Decrypt( fdata, fdata_dec, m_fileindexKey, m_fileindexKeyLength );
			fdata.Close();

			// Now same as FILEINDEX_ENTRY_COMPRESSED

			fsXor( info, fdata_dec );

			TMemoryStream fdata_raw;
			if( fsRle( fdata_dec, fdata_raw ) )
			{
				if( m_doExtraction ) fdata_raw.SaveToFile( fname.c_str() );

				printf(" ..done!\n");
			}
		
			// fsRle will display any errors

			fdata_dec.Close();
			fdata_raw.Close();

			break;
		}

		// Large files, some FSB (GunZ 2)
		case FILEINDEX_ENTRY_UNCOMPRESSED :
		{
			if( m_doExtraction )
				fdata.SaveToFile( fname.c_str() );

			puts(" ..done!");

			break;
		}

		default:
		{
			fdata.Close();
			printf("ERROR: Unknown compression type (%02X)\n", method);

			return false;
		}
	}

	fdata.Close();

	return true;
}

void z3ResEx::parseMsf( TMemoryStream &msf )
{
	switch( m_fileindexVer )
	{
		case 0 :
		{
			#define MAX_ERRORS 50

			unsigned char method( 0 );
			FILEINDEX_ENTRY info;
			char *strMRFN( nullptr );
			char *strName( nullptr );

			unsigned int items( 0 ), errors( 0 );

			while( ( msf.Position() < msf.Size() ) && ( errors < MAX_ERRORS ) )
			{
				method = msf.ReadByte();
				msf.Read( &info, sizeof( FILEINDEX_ENTRY ) );

				unpackStringEx( msf, strMRFN, info.lenMRFN );
				unpackStringEx( msf, strName, info.lenName );

				if( !( extractItem( info, method, strMRFN, strName ) ) )
					++errors;

				++items;

				delete strMRFN;
				delete strName;
			}

			break;
		}
		case 1 :
		{
			parseMsfMethod2( msf );

			break;
		}		
	}
}

