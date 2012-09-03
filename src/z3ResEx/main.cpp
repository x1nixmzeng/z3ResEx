/*
	z3ResEx
	Written by x1nixmzeng

	Initial version
*/

#include <stdio.h>

// Import TStream class
#include "mbuffer.h"

// Import fileindex definitions and encryption keys
#include "z3MSF.h"
#include "keys.h"

// Import methods
#include "methods.h"

bool readMSF( char *clientPath, TMemoryStream &msf )
{
	std::string fname;

	fname = clientPath;
	fname+= "fileindex.msf";

	TMemoryStream fileIndex, fileIndex_dec;

	if( !( fileIndex.LoadFromFile( fname.c_str() ) ) )
	{
		printf("ERROR: Unable to open file (%s)\n", fname.c_str() );
		fname.clear();
		return false;
	}

	if( fileIndex.Size() == 0 )
	{
		fileIndex.Close();
		printf("ERROR: File is empty (%s)\n", fname.c_str() );
		return false;
	}

	if( z3Decrypt( Z3_KEY_GUNZ2_NETMARBLE, fileIndex, fileIndex_dec ) )
	{
		fileIndex.Close();
		
		if( fsRle( fileIndex_dec, msf ) )
		{
			fileIndex_dec.Close();

			printf("MSF has been extracted (%u bytes)!\n", msf.Size());
		}

		fileIndex_dec.Close();
	}
	
	fileIndex.Close();
	fileIndex_dec.Close();

	return true;
}



void extractionMain( char *clientPath, int resolvedType )
{
	FILEINDEX_ENTRY info;
	unsigned char method;
	unsigned int items;

	TMemoryStream msf;

	if( readMSF( clientPath, msf ) )
	{
		items = 0;

		while( msf.Position() < msf.Size() )
		{
			method = msf.ReadByte();
			msf.Read( &info, sizeof( FILEINDEX_ENTRY ) );
			
			msf.Seek( info.lenMRFN + info.lenName, bufo_skip );

			/*
			if( !( extractItem( clientPath, info, method ) ) )
			{
				// error and break out ?
			}
			*/

			++items;
		}

		printf("Found %u items\n", items);
	}

	msf.Close();
}


int main( int argc, char **argv )
{
	printf("z3ResEx\nWritten by x1nixmzeng\n\n");

	bool doExtraction( false );

	extractionMain("", 0);

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
