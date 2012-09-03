/*
	mbuffer
	Wrapper for generic data streams (based on Delphi TMemoryStream)

	x1nixmzeng (August 2012)
*/
#include "mbuffer.h"

mbuffer::mbuffer( )
{
	buff = NULL;
	boff = 0;
	size = 0;
}

mbuffer::~mbuffer( )
{
	Close();
}

/*
	Clear the existing buffer data
*/
void mbuffer::Close()
{
	if( buff ) free( buff );
	buff = NULL;
	boff = 0;
	size = 0;
}

/*
	Get the buffer size
*/
unsigned int mbuffer::Size() const
{
	return( size );
}

/*
	Get the buffer offset
*/
unsigned int mbuffer::Position() const
{
	return( boff );
}

/*
	Calculate the buffer position (this is typically boff)
*/
void *mbuffer::bufferPosition( unsigned int pos ) const
{
#ifdef __linux__
	// g++ just doesn't care
	return( buff + pos );
#else
	return( (void *)((unsigned int)buff + pos) );
#endif
}

/*
	Write new data to the buffer
	** Previous data is cleared **
*/
bool mbuffer::DataWrite( void *data, unsigned int count )
{
	Close();

	if( !( buff = malloc( count ) ) )
		return false;

	boff = 0;
	size = count;

	memcpy( buff, data, count );
	return true;
}

/*
	Write data to the end of the buffer
	** Buffer position is not changed **
*/
bool mbuffer::DataAppend( void *data, unsigned int count )
{
	if( !( buff = realloc( buff, size + count ) ) )
		return false;

	memcpy( bufferPosition( size ), data, count );
	size += count;

	return true;
}

/*
	Attempt to update the buffer position
*/
bool mbuffer::Seek( unsigned int pos, buffer_origin from )
{
	switch( from )
	{
		case bufo_start :
		{
			// Check the new position is less than the buffer size
			if( pos > size ) return false;
			boff = pos;

			break;
		}

		case bufo_skip :
		{
			// Check the new position is less than the current position, plus the buffer size
			if( boff + pos > size ) return false;
			boff += pos;

			break;
		}

		case bufo_end :
		{
			// Check the new position is less than the buffer size
			if( pos > size ) return false;
			boff = size - pos;

			break;
		}

		default:
			return false;
	}

	return true;
}

/*
	Create buffer from existing data
	** This will create a new buffer every time **
*/
bool mbuffer::LoadFromBuffer( void *data, unsigned int count )
{
	return( DataWrite( data, count ) );
}

#ifdef MEMBUFFER_ALLOW_FILES
/*
	Create buffer from file data
	** Modified to use TFileStream **
*/
bool mbuffer::LoadFromFile( const char *filename )
{
	char *fileBuffer;
	bool result( false );
	TFileStream fileInfo( filename );

	if( fileInfo.isOpen() )
	{
		unsigned int fileSize( fileInfo.Size() );

		if( fileBuffer = (char *)malloc( fileSize ) )
		{
			fileInfo.Read( fileBuffer, fileSize );
			result = DataWrite( fileBuffer, fileSize );
			free( fileBuffer );
		}

		fileInfo.Close();
	}

	return( result );
}
#endif

/*
	Attempt to read data
*/
bool mbuffer::Read( void *data, unsigned int count )
{
	if( boff + count > size )
		return false;

	if( count > 0 )
	{
		if( count == 1 )
		{
			*(unsigned char *)data = *(unsigned char *)bufferPosition( boff );
		}
		else if( count == 2 )
		{
			*(unsigned short *)data = *(unsigned short *)bufferPosition( boff );
		}
		else if( count == 4 )
		{
			*(unsigned int *)data = *(unsigned int *)bufferPosition( boff );
		}
		else
		{
			memcpy( data, bufferPosition( boff ), count );
		}

		boff += count;
	}

	return true;
}

/*
	Attempt to write data
*/
bool mbuffer::Write( void *data, unsigned int count )
{
	if( Size() == Position() )
		return DataAppend( data, count );
	
	// else do something magical..
	return false;
}

unsigned char *mbuffer::Data( ) const
{
	return (unsigned char *)buff;
}


