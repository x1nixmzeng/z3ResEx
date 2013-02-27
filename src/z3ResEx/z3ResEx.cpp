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

void z3ResEx::setMessage( const char *msgFormat )
{
	sprintf_s( m_lastMsg, msgFormat );
}

void z3ResEx::setMessage( const char *msgFormat, const char *arg )
{
	sprintf_s( m_lastMsg, msgFormat, arg );
}

void z3ResEx::setMessage( const char *msgFormat, const unsigned int arg )
{
	sprintf_s( m_lastMsg, msgFormat, arg );
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
			setMessage( "ERROR: Cannot set current path to \"%s\"", args.getArgCStr(1) );
			return false;
		}

		if( args.hasArg("--usage") )
		{
			PrintUsage();

			// Also stop execution here (but successfully)
			return false;
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
	else
	{
		// No arguments, but inform user of them!
		puts("To see all options, use the --usage flag\n");
	}

	if( m_verboseMessages )
	{
		// Print current flags

		if( m_doExtraction )	puts("[Enabled] Extracting");
		if( m_listContents )	puts("[Enabled] Listing filesystem");
	}

	return true;
}

const char *z3ResEx::lastMessage( ) const
{
	return m_lastMsg;
}

std::string z3ResEx::fsRename( const char *strMrf, const char *strName ) const
{
	std::string name( "datadump/" );

	switch( m_fileindexVer )
	{
		case 0 :
		{
			// Append the MRF name
			name += strMrf;
			// Now remove the MRF extension (.mrf, .001, .002, etc)
			name = name.substr( 0, name.rfind('.') );
			// Append the filename
			name += "/";
			name += strName;
			break;
		}

		case 1 :
		{
			// Append the filename
			name += strName;
		}
	}
		
	// Correct any paths
	std::replace( name.begin(), name.end(), '\\', '/' );

	return name;
}

void z3ResEx::unpackStringEx( TMemoryStream &msf, unsigned char *&buf, const unsigned int len ) const
{
	buf = new unsigned char[ len +1 ];
	msf.Read( buf, len );
	buf[len] = 0;

	/*
		Simple xor added to strings in later clients
			buf[0] is the xor character
			buf[1] to buf[size-1] contains the xored string
	*/
	if( m_fileindexVer == 1 )
	{
		const unsigned char encKey( buf[0] );	// First byte is the key
		unsigned int i = 1;						// Second byte starts the data

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
		setMessage( "ERROR: Unable to open file (%s)", msfName );
	}
	else
	{
		TMemoryStream msf;

		m_fileindexKey			= nullptr;
		m_fileindexKeyLength	= 0;

		//
		// Brute-force the key (version 1)
		//

		unsigned int keyIndex( 0 );

		// For all known keys
		while( ( keyIndex < Z3_KEY_LIST_LENGTH ) && ( msf.Size() == 0 ) )
		{
			// Try to read the fileindex
			if( fsReadMSF( msf, Z3_KEY_LIST[ keyIndex ], Z3_KEY_LENGTH, 0 ) )
			{
				m_fileindexKey			= Z3_KEY_LIST[ keyIndex ];
				m_fileindexKeyLength	= Z3_KEY_LENGTH;
				m_fileindexVer			= 0;
			}

			++keyIndex;
		}

		// If key has not been found
		if( m_fileindexKey == nullptr )
		{
			//
			//  Continue to brute-force the key (version 2)
			//

			// Only one known key for this version
			if( fsReadMSF( msf, Z3_KEY_GUNZ2_METHOD2, Z3_KEY_LENGTH_METHOD2, 1 ) )
			{
				m_fileindexKey			= Z3_KEY_GUNZ2_METHOD2;
				m_fileindexKeyLength	= Z3_KEY_LENGTH_METHOD2;
				m_fileindexVer			= 1;
			}
		}

		// If a valid key has been found and fileindex loaded
		if( !( ( m_fileindexKey == nullptr ) && ( msf.Size() == 0 ) ) )
		{
			// Attempt to parse it (to extract or list files)
			msf.Seek( 0, bufo_start );
			parseMsf( msf );
		}
		else
		{
			// No key found or incompatiable file (not checked)
			setMessage( "ERROR: This file is using an updated key or unsupported method" );
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
			//  otherwise exactly the same method as 0
			
			fileIndex_dec.LoadFromBuffer( (void *)(fileIndex.Data() + 3), fileIndex.Size() -3 );
			fileIndex.Close();

			// Attempt to decrypt the data
			if( !( z3Decrypt( fileIndex_dec, fileIndex, key, keylen ) ) )
			{
				fileIndex_dec.Close();
				return false;
			}

			// Attempt to uncompress the data
			if( !( fsRle( fileIndex, msf, true ) ) )
			{
				fileIndex.Close();
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
		//printf("ERROR: Problems decoding RLE buffer size\n");
		return false;
	}

	if( isMSF && !( msfSizeFlag == expectedSize ) )
	{
		dst.Close();
		//printf("ERROR: Unexpected MSF buffer size\n");
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
			//printf("ERROR: Problems decoding RLE buffer\n");

			return false;
		}

		pData += len;
	}

	dst.Write( tmpBuffer, expectedSize );

	delete tmpBuffer;

	return true;
}

void z3ResEx::fsXor( TMemoryStream &src, unsigned int key ) const
{
	z3Xor::rs3Unscramble( src.Data(), src.Size(), key );
}

void z3ResEx::parseMsfMethod2( TMemoryStream &msf )
{
	unsigned short strLen( 0 );
	unsigned char *strBuffer( nullptr );

	unsigned short mrfIndexLen( 0 );

	// Folders are now in a table at the top of the file
	msf.Read( &mrfIndexLen, sizeof( unsigned short ) );

	// List of filenames
	//typedef std::pair<string, TFileStream* > mrfItem;
	typedef vector<string > mrfItemList;

	mrfItemList vecMsf;
	vecMsf.resize( mrfIndexLen );
			
	// MRF filenames are now packed in a list
	for( unsigned short i( 0 ); i != mrfIndexLen; ++i )
	{
		strLen = msf.ReadUShort();
		unpackStringEx( msf, strBuffer, strLen );

		// Required to rename files
		//vecMsf[i].first.assign( (char *)strBuffer );
		// Cached file opening (and a pointer so we can call the constructor)
		//vecMsf[i].second = new TFileStream( strBuffer );

		vecMsf[i].assign( (char *)strBuffer );

		delete strBuffer;
	}

	// Files are now listed (similar to before)
	FILEINDEX_ENTRY2 fiItem;

	unsigned int items( 0 ), errors( 0 );

	//msf.SaveToFile("debugFilesys.dat");

	while( ( msf.Position() < msf.Size() ) && ( errors < MAX_ERRORS ) )
	{
		msf.Read( &fiItem, sizeof( FILEINDEX_ENTRY2 ) );

		strLen = msf.ReadUShort();
		unpackStringEx( msf, strBuffer, strLen );
		
		if( m_listContents )
		{
			printf( "%s (%u bytes)\n", strBuffer, fiItem.size );
		}
		else
		{
			if( !( extractItem2( fiItem, vecMsf[ fiItem.mrfIndex ], (char *)strBuffer ) ) )
				++errors;
		}

		delete strBuffer;

		++items;
	}

	vecMsf.clear();

	printf( "Processed %u items (%u issues)\n\n", items, errors );
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


bool z3ResEx::extractItem2( FILEINDEX_ENTRY2 &info, const string &strMrf, char *strName )
{
	TFileStream mrf( strMrf.c_str() );

	if( !( mrf.isOpen() ) )
	{
		setMessage( "ERROR: Unable to open file (%s)", strMrf.c_str() );
		return false;
	}

	// Format the output filename
	std::string fname( fsRename( strMrf.c_str(), strName ) );
	
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

	printf(	"Saving %s.. ",	fname.substr( fname.rfind('/') +1 ).c_str() );

	fsCreatePath( fname );

	switch( info.type )
	{
		case FILEINDEXITEM_COMPRESSED :
		{
			fsXor( fdata, info.xorkey );

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

		/*
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
				if( m_doExtraction )
					fdata_raw.SaveToFile( fname.c_str() );

				printf(" ..done!\n");
			}
		
			// fsRle will display any errors

			fdata_dec.Close();
			fdata_raw.Close();

			break;
		}
		*/

		case FILEINDEXITEM_UNCOMPRESSED :
		{
			fdata.SaveToFile( fname.c_str() );

			puts(" ..done!");

			break;
		}

		default:
		{
			fdata.Close();
			printf("ERROR: Unknown compression type (%02X)\n", info.type);

			return false;
		}
	}

	fdata.Close();

	return true;
}



bool z3ResEx::extractItem( FILEINDEX_ENTRY &info, unsigned char method, char *strMrf, char *strName )
{
	TFileStream mrf( strMrf );

	if( !( mrf.isOpen() ) )
	{
		setMessage( "ERROR: Unable to open file (%s)", strMrf );
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
			fsXor( fdata, info.xorkey );

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

			fsXor( fdata_dec, info.xorkey );

			TMemoryStream fdata_raw;
			if( fsRle( fdata_dec, fdata_raw ) )
			{
				if( m_doExtraction )
					fdata_raw.SaveToFile( fname.c_str() );

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
			unsigned char method( 0 );
			FILEINDEX_ENTRY info;
			unsigned char *strMRFN( nullptr );
			unsigned char *strName( nullptr );

			unsigned int items( 0 ), errors( 0 );

			while( ( msf.Position() < msf.Size() ) && ( errors < MAX_ERRORS ) )
			{
				method = msf.ReadByte();
				msf.Read( &info, sizeof( FILEINDEX_ENTRY ) );

				unpackStringEx( msf, strMRFN, info.lenMRFN );
				unpackStringEx( msf, strName, info.lenName );

				if( m_listContents )
				{
					printf( "%s (%u bytes)\n", strName, info.size );
				}
				else
				{
					if( !( extractItem( info, method, (char *)strMRFN, (char *)strName ) ) )
						++errors;
				}

				++items;

				delete strMRFN;
				delete strName;
			}

			printf( "Processed %u items (%u issues)\n\n", items, errors );

			break;
		}
		case 1 :
		{
			parseMsfMethod2( msf );

			break;
		}		
	}
}

