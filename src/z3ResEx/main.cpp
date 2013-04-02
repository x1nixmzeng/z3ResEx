/*
	z3ResEx
	Written by x1nixmzeng

	main.cpp
	Entry point for the z3ResEx project
*/

#include <stdio.h>
#include "targs.h"
#include "z3ResEx.h"

int main( int argc, char **argv )
{
	targs params( argc, argv );
	z3ResEx app;
	
	puts( "z3ResEx" );
	puts( "Researched and coded by x1nixmzeng\n" );
	puts( "Last updated " __DATE__ "\n" );

	// Push commandline arguments
	if( app.setFlags( params ) )
	{
		// Try running
		app.Run();
	}

	puts( app.lastMessage( ) );

	return 0;
}
