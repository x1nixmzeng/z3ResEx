#include "fbuffer.h"

/*
	Opens a file for reading only
	** No support for writing to file yet **
*/
fbuffer::fbuffer( const char *filename )
{
	lastSize = 0;
	if( fHandle = fopen( filename, "rb+" ) )
	{
		Seek( 0, bufo_end );
		lastSize = Position();
		Seek( 0, bufo_start );
	}
}

fbuffer::fbuffer( const char *filename, bool recreate )
{
	lastSize = 0;

	if( fHandle = fopen( filename, ( recreate ? "wb+" : "rb+" ) ) )
	{
		Seek( 0, bufo_end );
		lastSize = Position();
		Seek( 0, bufo_start );
	}
}

fbuffer::~fbuffer( )
{
	Close();
}

/*
	Public check to verify the file open successfully
*/
bool fbuffer::isOpen() const
{
	return( !( fHandle == NULL ) );
}

/*
	Return the cached filesize
*/
unsigned int fbuffer::Size() const
{
	return( lastSize );
}

/*
	Get the current file position
	** No checks on invalid file handles **
*/
unsigned int fbuffer::Position() const
{
	return( isOpen() ? ftell( fHandle ) : 0 );
}

/*
	Close the file handle if it exists
*/
void fbuffer::Close()
{
	if( isOpen() )
		fclose( fHandle );
	fHandle = NULL;
}

/*
	Attempt to update the file position
*/
bool fbuffer::Seek( unsigned int pos, buffer_origin from )
{
	if( !( isOpen() ) )
		return false;

	switch( from )
	{
		case bufo_start :
		{
			if( pos > Size() ) return false;
			fseek( fHandle, pos, SEEK_SET );

			break;
		}

		case bufo_skip :
		{
			if( Position() + pos > Size() ) return false;
			fseek( fHandle, pos, SEEK_CUR );

			break;
		}

		case bufo_end :
		{
			if( pos > Size() ) return false;
			fseek( fHandle, pos, SEEK_END );
			break;
		}

		default:
			return false;
	}

	return true;
}

/*
	Attempt to read data
	** No checks are made on how much data can be read out **
*/
bool fbuffer::Read( void *outBuf, unsigned int size )
{
	if( !( isOpen() ) )
		return false;

	return( fread( outBuf, 1, size, fHandle ) == size );
}

/*

*/
bool fbuffer::Write( void *srcBuf, unsigned int size )
{
	if( !( isOpen() ) )
		return false;

	return( fwrite( srcBuf, 1, size, fHandle ) == size );
}