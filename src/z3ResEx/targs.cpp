#include "targs.h"

targVectorCIt targs::GetItemByName( const string &sItemName ) const
{
	return std::find( m_argVector.begin(), m_argVector.end(), sItemName );
}

targs::targs( int argc, char **argv )
{
	for( int i( 0 ); i < argc; ++i )
		m_argVector.push_back( argv[ i ] );
}

targs::~targs( )
{
	m_argVector.clear();
}

unsigned int targs::Count( ) const
{
	return m_argVector.size();
}

bool targs::HasItem( const string &sItemName ) const
{
	return( !( GetItemByName( sItemName ) == m_argVector.end() ) );
}

const char *targs::GetItemValue( const string &sItemName ) const
{
	if( HasItem( sItemName ) )
	{
		targVectorCIt argItem = GetItemByName( sItemName );
		argItem++;

		if( !( argItem == m_argVector.end() ) )
		{
			return argItem->c_str();
		}
	}

	return NULL;
}

const char *targs::GetItemValue( unsigned int uItemIndex ) const
{
	if( uItemIndex < Count() )
	{
		return m_argVector.at(uItemIndex).c_str();
	}

	return NULL;
}
