#include "cstring_util.h"
#include <string.h>

tokType_t StrToken( const char* c )
{
	static const char syms[] =
	{
		'\t', ' ', '\n', '\r',
		'*', '[', ']', '(', ')'
	};

	// If we have an indent, space, newline, or a comment, then the token is invalid
	if ( *c == '/' && *( c + 1 ) == '/' )
		return TOKTYPE_COMMENT;

	for ( int i = 0; i < SIGNED_LEN( syms ); ++i )
		if ( *c == syms[ i ] )
			return TOKTYPE_GENERIC;

	return TOKTYPE_VALID;
}

const char* StrNextLine( const char* buffer )
{
	while ( *buffer != '\n' )
		buffer++;

	return buffer;
}

const char* StrSkipInvalid( const char* buffer )
{
	tokType_t tt;
	while ( ( tt = StrToken( buffer ) ) != TOKTYPE_VALID )
	{
		if ( tt == TOKTYPE_COMMENT )
			buffer = StrNextLine( buffer );
		else
			buffer++;
	}

	return buffer;
}

const char* StrReadToken( char* out, const char* buffer )
{
	buffer = StrSkipInvalid( buffer );

	// Parse token
	char* pOut = out;
	while ( StrToken( buffer ) == TOKTYPE_VALID )
	{
		if ( !*buffer )
		{
			break;
		}

		*pOut++ = *buffer++;
	}

	return buffer;
}

const char* StrNextNumber( const char* buffer )
{
	while ( !isdigit( *buffer ) )
	{
		if ( *buffer == '-' && isdigit( *( buffer + 1 ) ) )
		{
			break;
		}

		buffer++;
	}

	return buffer;
}

void StrLower( char * str )
{
	char* p = str;

	while ( *p )
	{
		*p = tolower( *p );
		p++;
	}
}

void StrLowerN( char* str, size_t maxLength )
{
	char* p = str;

	while ( *p && ( size_t )( p - str ) < maxLength )
	{
		*p = tolower( *p );
		p++;
	}
}

size_t StrFindLastOf( const char* str, const char* ch )
{
	const char * result = nullptr;

	size_t index = STRING_NPOS;

	do 
	{
		result = strstr( str + index + 1, ch );

		if ( result )
		{
			index = ( size_t )( result - str );
		}
	}
	while ( result );

	return index; 
}

void StrFixupAssetPath( char* assetPath )
{
	StrLowerN( assetPath, StrFindLastOf( assetPath, "/" ) );
}

float StrReadFloat( const char*& buffer )
{
	char f[ 24 ];
	memset( f, 0, sizeof( f ) );
	buffer = StrReadToken( f, buffer );
	return ( float ) strtod( f, NULL );
}
