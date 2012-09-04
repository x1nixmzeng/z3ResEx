/*
	fbuffer
	Wrapper for larger data stream, direct from file (based on Delphi TFileStream)

	x1nixmzeng (August 2012)
*/

#ifndef _H_FBUFFER_
#define _H_FBUFFER_

#include <stdio.h>

#ifdef _WIN32
	#include <windows.h>
#elif __linux__
	#include <stdlib.h>
#endif

#include "xbuffer.h"

class fbuffer: public xbuffer
{
private:
	FILE *fHandle;
	unsigned int lastSize;
public:
	fbuffer( const char * );
	fbuffer( const char *, bool );
	~fbuffer( );

	bool isOpen() const;

	unsigned int Size() const;
	unsigned int Position() const;

	void Close();

	bool Seek( unsigned int, buffer_origin );

	bool Read( void *, unsigned int );
	bool Write( void *, unsigned int );
};

typedef fbuffer TFileStream;

/*
bool FileExists( const char *filename )
{
	TFileStream tmpFs( filename );
	return( tmpFs.isOpen() );
}*/

unsigned int TFileSize( const char * );

#endif
