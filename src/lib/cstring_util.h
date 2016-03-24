#pragma once

#include "common.h"

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

float StrReadFloat( const char*& buffer );
