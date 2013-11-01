/*
	z3ResEx
	Written by x1nixmzeng

	targs.h
	Basic class to handle program parameters
*/

#ifndef TARGS_H
#define TARGS_H

#include <string>
#include <vector>

using std::string;
using std::vector;

#include <algorithm>

typedef vector<string >				targVector;
typedef targVector::iterator		targVectorIt;
typedef targVector::const_iterator	targVectorCIt;

class targs
{
private:
	targVector		m_argVector;

	targVectorCIt	GetItemByName( const string &sItemName ) const;

public:
	// Constructor
	targs( int, char ** );
	~targs();

	// Accessors
	unsigned int	Count( ) const;
	bool			HasItem( const string &sItemName ) const;
	const char *	GetItemValue( const string &sItemName ) const;
	const char *	GetItemValue( unsigned int uItemIndex ) const;
};

#endif
