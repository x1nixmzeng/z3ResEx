/*
	mbuffer
	Wrapper for generic data streams (based on Delphi TMemoryStream)

	> Modified to load from file using TFileStream
	> Adapted for linux (Aug12-2012)

	x1nixmzeng (August 2012)
*/
#ifndef _H_MEMBUFFER_
#define _H_MEMBUFFER_

#define MEMBUFFER_ALLOW_FILES

#include <stdio.h>

#ifdef _WIN32
	#include <Windows.h>
#elif __linux__
	#include <stdlib.h>
	#include <string.h> // for memcpy
#endif

#include "xbuffer.h"
#ifdef MEMBUFFER_ALLOW_FILES
	#include "fbuffer.h"	
#endif

class mbuffer: public xbuffer
{
private:
	void *buff;
	unsigned int size, boff;

	bool DataWrite( void *, unsigned int );
	bool DataAppend( void *, unsigned int );

	void * bufferPosition( unsigned int ) const;

public:
	mbuffer( );
	~mbuffer( );

	unsigned int Size() const;
	unsigned int Position() const;

	void Close();

	bool Seek( unsigned int, buffer_origin );

	bool LoadFromBuffer( void *, unsigned int );
#ifdef MEMBUFFER_ALLOW_FILES
	bool LoadFromFile( const char *filename );
	bool SaveToFile( const char *filename ) const;
#endif

	bool Read( void *, unsigned int );
	bool Write( void *, unsigned int );

	unsigned char *Data( ) const; // riske
};

typedef mbuffer TMemoryStream;

#endif
