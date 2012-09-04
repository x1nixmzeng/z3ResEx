/*
	z3ResEx
	Written by x1nixmzeng

	main.cpp

	Initial version
	Added basic extraction resuming (when file exists and filesize matches)
*/

#include <stdio.h>

// Import TStream class
#include "mbuffer.h"
#include "fbuffer.h"

// Import fileindex definitions and encryption keys
#include "z3MSF.h"
#include "keys.h"

// Import data manipulation methods
#include "methods.h"


unsigned char *z3CurrentKey( nullptr );


/* todo: move this to xbuffer? */
void fsCreatePath( std::string &strPath )
{
	int pathLoc( strPath.find('/') );

	while( !( pathLoc == std::string::npos ) )
	{
		CreateDirectoryA( strPath.substr( 0, pathLoc ).c_str(), nullptr );

		pathLoc = strPath.find( '/', pathLoc+1 );
	}
}

std::string fsRename( char *strMrf, char *strName )
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

bool extractItem( FILEINDEX_ENTRY &info, unsigned char method, char *strMrf, char *strName )
{
	TFileStream mrf( strMrf );

	if( !( mrf.isOpen() ) )
	{
		printf("ERROR: Could not open file (%s)\n", strMrf );
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

	printf("Saving %s.. ", fname.substr( fname.rfind('/') +1 ).c_str() );

	fsCreatePath( fname );

	switch( method )
	{
		case FILEINDEX_ENTRY_COMPRESSED :
		{
			fsXor( info, fdata );

			TMemoryStream fdata_raw;
			if( fsRle( fdata, fdata_raw, false ) )
			{
				fdata_raw.SaveToFile( fname.c_str() );
				printf("done!\n");
			}
		
			// fsRle will display any errors

			fdata_raw.Close();
			break;
		}

		case FILEINDEX_ENTRY_COMPRESSED2 :
		{
			TMemoryStream fdata_dec;
			z3Decrypt( z3CurrentKey, fdata, fdata_dec );
			fdata.Close();

			// Now same as FILEINDEX_ENTRY_COMPRESSED

			fsXor( info, fdata_dec );

			TMemoryStream fdata_raw;
			if( fsRle( fdata_dec, fdata_raw, false ) )
			{
				fdata_raw.SaveToFile( fname.c_str() );
				printf("done!\n");
			}
		
			// fsRle will display any errors

			fdata_dec.Close();
			fdata_raw.Close();

			break;
		}

		case FILEINDEX_ENTRY_UNCOMPRESSED :
		{
			std::string fname( fsRename( strMrf, strName ) );
			fsCreatePath( fname );

			fdata.SaveToFile( fname.c_str() );
			printf("done!\n");

			break;
		}

		default:
		{
			fdata.Close();
			printf("ERROR: Unknown compression type (%s)\n", strName);

			return false;
		}
	}

	fdata.Close();

	return true;
}

void extractionMain( unsigned char *z3Key )
{
	TMemoryStream msf;

	// Check the fileindex can be located and decrypted
	if( fsReadMSF( msf, z3Key ) )
	{
		unsigned int items( 0 ), errors( 0 );

		FILEINDEX_ENTRY info;
		unsigned char method;

		char *strMRFN, *strName;

		#define unpackString(buf,len) \
		{ \
			buf = new char[ len +1 ]; \
			msf.Read( buf, len ); \
			buf[ len ] = 0; \
		}

		while( ( msf.Position() < msf.Size() ) && ( errors < 10 ) )
		{
			method = msf.ReadByte();
			msf.Read( &info, sizeof( FILEINDEX_ENTRY ) );

			unpackString( strMRFN, info.lenMRFN );
			unpackString( strName, info.lenName );

			if( !( extractItem( info, method, strMRFN, strName ) ) )
				++errors;

			++items;

			delete strMRFN;
			delete strName;
		}

		if( errors > 9 )
			printf("ERROR: Extraction stopped as there were too many errors\n");
		else
			printf("Extracted %u files (%u problems)\n", items, errors);
	}

	msf.Close();
}


int main( int argc, char **argv )
{
	printf("z3ResEx\nWritten by x1nixmzeng\n\n");

	/*
		Possible arguments

		REQ
			Client version
		Client path (i.e not current directory)
		Verbose output
	*/

	// Brute force the key
	{
		unsigned int keyIndex( 0 );
		TMemoryStream msf;

		while( ( keyIndex < Z3_KEY_LIST_LENGTH ) && ( msf.Size() == 0 ) )
		{
			if( fsReadMSF( msf, Z3_KEY_LIST[ keyIndex ] ) )
				z3CurrentKey = Z3_KEY_LIST[ keyIndex ];

			++keyIndex;
		}

		if( !( z3CurrentKey == nullptr ) )
		{
			printf("Found key (%u)!\n", keyIndex );
		}
		else
		{
			printf("Sorry, no key was found\n");
		}
		

	}

	return 0;
}
