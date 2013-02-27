#include "targs.h"

targVectorCIt targs::getArg( const string &str ) const
{
	return std::find( m_args.begin(), m_args.end(), str );
}

bool targs::hasArg( const string &str ) const
{
	return( !( getArg( str ) == m_args.end() ) );
}

targs::targs( int argc, char **argv )
{
	for( int i( 0 ); i < argc; ++i )
		m_args.push_back( argv[ i ] );
}

targs::~targs( )
{
	m_args.clear();
}

const char *targs::getArgCStr( unsigned int index ) const
{
	if( index >= m_args.size() )
		return NULL;

	return m_args.at( index ).c_str();
}

unsigned int targs::count( ) const
{
	return m_args.size();
}