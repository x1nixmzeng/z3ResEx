/*
	z3ResEx
	Written by x1nixmzeng

	main.cpp

	Initial version
*/

#include <stdio.h>

// Import TStream class
#include "mbuffer.h"
#include "fbuffer.h"

// Import fileindex definitions and encryption keys
#include "z3MSF.h"
#include "keys.h"

// Import methods
#include "methods.h"

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
	// Load MRF

	TFileStream mrf( strMrf );

	if( !( mrf.isOpen() ) )
	{
		printf("ERROR: Could not open file (%s)\n", strMrf );
		return false;
	}

	mrf.Seek( info.offset, bufo_start );

	unsigned char *buf( new unsigned char[ info.zsize ] );
	mrf.Read( buf, info.zsize );
	mrf.Close();

	TMemoryStream fdata;
	fdata.LoadFromBuffer( buf, info.zsize );

	delete buf;

	if( method == FILEINDEX_ENTRY_COMPRESSED )
	{
		z3Xor::rs3Unscramble( fdata.Data(), fdata.Size(), info.xorkey );

		TMemoryStream fdata_raw;
		if( fsRle( fdata, fdata_raw, false ) )
		{
			printf("Got file data!\n");

			// todo: save it
		}


		fdata_raw.Close();
		
	}

	fdata.Close();

	return true;
}

void extractionMain( int resolvedType )
{
	TMemoryStream msf;

	// Check the fileindex can be located and decrypted
	if( fsReadMSF( msf ) )
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
			{
				++errors;
			}

			++items;

			delete strMRFN;
			delete strName;
		}

		if( errors > 9 )
			printf("ERROR: Failed to extract too many files.\n");
		else
			printf("Found %u items\n", items);
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


	bool doExtraction( false );
	
	extractionMain( 0);

	if( argc == 2 )
	{
		
	}

	/*

	if( !( doExtraction ) )
	{
		printf(
			"Usage:\n" \
			"  z3resex.exe <game_id> <game_path> [-vd]\n\n" \
			"Supported game_id:\n"
		);

		// see keys.h
		for(int i=0;i<4;++i)
			printf("  %s (%s)\n", game_ids[i].z3Option, game_ids[i].z3Game);

		printf("\n");
	}

	*/

	return 0;
}
