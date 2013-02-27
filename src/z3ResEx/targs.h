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
	targVector m_args;

	targVectorCIt getArg( const string& ) const;

public:
	// Constructor
	targs( int, char ** );
	~targs();

	// Accessors
	unsigned int count( ) const;
	bool hasArg( const string& ) const;
	const char *getArgCStr( unsigned int ) const;
	
};

#endif
