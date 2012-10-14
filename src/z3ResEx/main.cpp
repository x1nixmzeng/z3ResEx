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

// Global pointer to current key
unsigned char *z3CurrentKey( nullptr );

#ifdef _DEBUG
	//#define SAVE_MSF_FILEINDEX
	#define DO_NOT_SAVE_DATA
	//#define VERBOSE
#endif

// Global option flags

bool user_opt_allow_extraction(
	#if defined( _DEBUG ) && defined( DO_NOT_SAVE_DATA )
		false
	#else
		true
	#endif
);

bool user_opt_list_files( false );

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
	
#ifdef VERBOSE
	printf("Using filename: %s\n", fname.c_str());
#endif

	// UNFORCED EXTRACTION
	// If file already exists, ignore it
	if( TFileSize( fname.c_str() ) == info.size )
	{
		mrf.Close();
		return true;
	}

	unsigned char *buf( new unsigned char[ info.zsize ] );

#ifdef VERBOSE
	printf("Allocated %u bytes\n", info.zsize);
#endif

	// Load MRF data into buffer
	mrf.Seek( info.offset, bufo_start );
	mrf.Read( buf, info.zsize );
	mrf.Close();

#ifdef VERBOSE
	printf("Read %u bytes at %u\n", info.zsize, info.offset );
#endif

	// Copy into TStream
	TMemoryStream fdata;
	fdata.LoadFromBuffer( buf, info.zsize );
	delete buf;

	printf
	(	
		( user_opt_allow_extraction ? "Saving %s.. " : "Checking %s.. " ),
		fname.substr( fname.rfind('/') +1 ).c_str()
	);

	// Create path only when extraction is flagged
	if( user_opt_allow_extraction )
		fsCreatePath( fname );

	switch( method )
	{
		// Compressed, most files
		case FILEINDEX_ENTRY_COMPRESSED :
		{
			fsXor( info, fdata );
		#ifdef VERBOSE
			printf("Complete XOR routine\n");
		#endif

			TMemoryStream fdata_raw;
			if( fsRle( fdata, fdata_raw ) )
			{
			#ifdef VERBOSE
				printf("Completed RLE routine\n");
			#endif
			
				if( user_opt_allow_extraction )
					fdata_raw.SaveToFile( fname.c_str() );

				printf("done!\n");
			}
		
			// fsRle will display any errors

			fdata_raw.Close();
			break;
		}

		// Encrypted and compressed, some system data (GunZ 2)
		case FILEINDEX_ENTRY_COMPRESSED2 :
		{
			TMemoryStream fdata_dec;
			z3Decrypt( z3CurrentKey, fdata, fdata_dec );
			fdata.Close();

			// Now same as FILEINDEX_ENTRY_COMPRESSED

			fsXor( info, fdata_dec );

			TMemoryStream fdata_raw;
			if( fsRle( fdata_dec, fdata_raw ) )
			{
				if( user_opt_allow_extraction )
					fdata_raw.SaveToFile( fname.c_str() );

				printf("done!\n");
			}
		
			// fsRle will display any errors

			fdata_dec.Close();
			fdata_raw.Close();

			break;
		}

		// Large files, some FSB (GunZ 2)
		case FILEINDEX_ENTRY_UNCOMPRESSED :
		{
			if( user_opt_allow_extraction )
				fdata.SaveToFile( fname.c_str() );

			printf("done!\n");

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

void extractionMain( TMemoryStream &msf )
{
	const unsigned int MAX_ERRORS( 50 );
	unsigned int items( 0 );

	FILEINDEX_ENTRY info;
	unsigned char method;

	char *strMRFN( nullptr ), *strName( nullptr );

	#define unpackString(buf,len) \
	{ \
		buf = new char[ len +1 ]; \
		msf.Read( buf, len ); \
		buf[ len ] = 0; \
	}

#ifdef SAVE_MSF_FILEINDEX
	msf.SaveToFile("z3debug_fileindex.msf");
#endif

	// Are we just listing files?
	if( user_opt_list_files )
	{
		std::string fname;

		printf("Listing filesystem contents\n\n");

		while( msf.Position() < msf.Size() )
		{
			method = msf.ReadByte();
			msf.Read( &info, sizeof( FILEINDEX_ENTRY ) );

			unpackString( strMRFN, info.lenMRFN );
			unpackString( strName, info.lenName );

			fname = fsRename( strMRFN, strName );
			printf("%s\n", fname.c_str());
			
			++items;

			delete strMRFN;
			delete strName;
		}

		fname.clear();
		printf("\nLocated %u files\n", items);
	}
	else
	// Run the main extraction loop
	{
		unsigned int errors( 0 );

		printf("Extracting filesystem contents\n\n");

		while( ( msf.Position() < msf.Size() ) && ( errors < MAX_ERRORS ) )
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

		if( errors >= MAX_ERRORS )
			printf("ERROR: Extraction stopped as there were too many errors\n");
		else
			printf("\nExtracted %u files (%u problems)\n", items, errors);
	}
}


int main( int argc, char **argv )
{
	printf
	(
		"z3ResEx" \
		"\nResearched and coded by x1nixmzeng\n\n"
	);
		
	// Check arguments
	if( argc > 1 )
	{
		if( SetCurrentDirectory( argv[1] ) == 0 )
		{
			printf("ERROR: Failed to set the client path (%s)\n", argv[1] );
			return 0;
		}

		if( argc > 2 )
		{
			// For all other arguments, check against known flags

			if( argv[2][0] == '-' )
			{
				// -v		Verbose
				// todo

				// -l		List all files
				if( argv[2][1] == 'l' )
				{
					user_opt_list_files = true;
				}
				else
				
				// -x		No extraction
				if( argv[2][1] == 'x' )
				{
					user_opt_allow_extraction = false;
				}

				// -f		Extract only (filter)
				// todo
			}

		}
	}

	// Check the fileindex exists
	if( TFileSize( msfName ) == 0 )
	{
		printf("ERROR: Unable to open file (%s)\n", msfName);
	}
	else
	{
		unsigned int keyIndex( 0 );
		TMemoryStream msf;

		// Brute-force the key
		while( ( keyIndex < Z3_KEY_LIST_LENGTH ) && ( msf.Size() == 0 ) )
		{
			if( fsReadMSF( msf, Z3_KEY_LIST[ keyIndex ] ) )
			{
				z3CurrentKey = Z3_KEY_LIST[ keyIndex ];
				
				// todo: verbose? - show key
			}

			++keyIndex;
		}

		if( !( z3CurrentKey == nullptr ) )
		{
			// Run main extraction loop
			if( !( user_opt_allow_extraction ) )
				printf("NOTE:  Opted NOT to save data\n");

			extractionMain( msf );
		}
		else
		{
			// No key found or incompatiable file (not checked)
			printf("ERROR: This file is using an updated key or unsupported method\n");
		}

		msf.Close();
	}

	return 0;
}
