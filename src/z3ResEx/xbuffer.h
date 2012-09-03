/*
	Generic TStream class

	x1nixmzeng (August 2012)
*/

#ifndef _H_XBUFFER_
#define _H_XBUFFER_

enum buffer_origin
{
	bufo_start,
	bufo_skip,
	bufo_end
};

class xbuffer
{
public:
	virtual ~xbuffer( ){};

	virtual unsigned int Size( ) const = 0;
	virtual unsigned int Position( ) const = 0;

	// ** Renamed from Clear() **
	virtual void Close( ) = 0;

	virtual bool Seek( unsigned int, buffer_origin ) = 0;

	virtual bool Read( void *, unsigned int ) = 0;


	int ReadInt()
	{ int tmp; Read( &tmp, sizeof( tmp ) ); return tmp; }

	unsigned int ReadUInt()
	{ unsigned int tmp; Read( &tmp, sizeof( tmp ) ); return tmp; }

	char ReadByte()
	{ char tmp; Read( &tmp, sizeof( tmp ) ); return tmp; }

	unsigned char ReadUByte()
	{ unsigned char tmp; Read( &tmp, sizeof( tmp ) ); return tmp; }

	short ReadShort()
	{ short tmp; Read( &tmp, sizeof( tmp ) ); return tmp; }

	unsigned short ReadUShort()
	{ unsigned short tmp; Read( &tmp, sizeof( tmp ) ); return tmp; }
};

typedef xbuffer TStream;

#endif
