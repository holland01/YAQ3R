#pragma once

#include "common.h"

#define STRING_NPOS 0xFFFFFFFF

enum tokType_t
{
	TOKTYPE_VALID = 0,
	TOKTYPE_GENERIC, // newlines, indents, whitespace, etc.
	TOKTYPE_COMMENT
};

tokType_t StrToken( const char* c );

const char* StrNextLine( const char* buffer );

const char* StrSkipInvalid( const char* buffer );

const char* StrReadToken( char* out, const char* buffer );

const char* StrNextNumber( const char* buffer );

void StrLower( char* str );

// Will stop at null term if hit before maxLength is reached
void StrLowerN( char* str, size_t maxLength );

// returns std::string::npos if nothing is found
size_t StrFindLastOf( const char* str, const char* ch );

float StrReadFloat( const char*& buffer );
