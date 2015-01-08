#include "effect_shader.h"
#include "q3bsp.h"
#include "log.h"

enum tokType_t
{
	TOKTYPE_VALID = 0,
	TOKTYPE_GENERIC, // newlines, indents, whitespace, etc.
	TOKTYPE_COMMENT
};

static INLINE GLenum GL_EnumFromStr( const char* str )
{
	if ( strcmp( str, "GL_SRC_ALPHA" ) == 0 )
		return GL_SRC_ALPHA;
	
	if ( strcmp( str, "GL_ONE_MINUS_SRC_ALPHA" ) == 0 )
		return GL_ONE_MINUS_SRC_ALPHA;

	return 0;
}

static INLINE tokType_t Token( const char* c )
{
	// If we have an indent, space, newline, or a comment, then the token is invalid
	if ( *c == '/' && *( c + 1 ) == '/' )
		return TOKTYPE_COMMENT;

	if ( *c == '\t' || *c == ' ' || *c == '\n' )
		return TOKTYPE_GENERIC;

	return TOKTYPE_VALID;
}

static INLINE const char* NextLine( const char* buffer )
{
	while ( *buffer != '\n' )
		buffer++;

	return buffer;
}

static INLINE const char* SkipInvalid( const char* buffer )
{
	tokType_t tt;
	while ( ( tt = Token( buffer ) ) != TOKTYPE_VALID )
	{
		if ( tt == TOKTYPE_COMMENT )
			buffer = NextLine( buffer );
		else
			buffer++;
	}

	return buffer;
}

static const char* ReadToken( char* out, const char* buffer )
{
	buffer = SkipInvalid( buffer );

	// Parse token
	int charCount = 0;
	char* pOut = out;
	while ( Token( buffer ) == TOKTYPE_VALID )
	{
		*pOut++ = *buffer++;
		charCount++;
	}

	return buffer;
}

// Returns the char count to increment the filebuffer by
static const char* ParseEntry( shaderInfo_t* outInfo, mapData_t* map, const char* buffer, const int level )
{
	buffer = SkipInvalid( buffer );

	// Begin stage?
	if ( *buffer == '{' )
		return ParseEntry( outInfo, map, buffer + 1, level + 1 );

	// End stage; we done
	if ( *buffer == '}' )
	{
		// We're back out into the main level, so we're finished with this entry.
		if ( level == 1 )
		{
			return buffer + 1;
		}
		// We're not in the main level, but we're leaving this stage, so decrease our level by 1 and add on to our stageCount
		else
		{
			outInfo->stageCount += 1;
			return ParseEntry( outInfo, map, buffer + 1, level - 1 );
		}
	}

	// No invalid tokens ( e.g., spaces, indents, comments, etc. ); so, this must be a header
	if ( level == 0 )
	{
		// Find our entity string
		char entCandidate[ SHADER_MAX_TOKEN_CHAR_LENGTH ];
		memset( entCandidate, 0, sizeof( entCandidate ) );

		buffer = ReadToken( outInfo->name, buffer );	
		buffer = ParseEntry( outInfo, map, buffer + 1, level );
	}
	else
	{
		char token[ SHADER_MAX_TOKEN_CHAR_LENGTH ];
		memset( token, 0, sizeof( token ) );

		buffer = ReadToken( token, buffer );

		// Evaluate possible global parameters
		if ( strcmp( token, "surfaceparm" ) == 0 )
		{
			char value[ SHADER_MAX_TOKEN_CHAR_LENGTH ] = {}; 

			buffer = ReadToken( value, buffer );
			
			if ( strcmp( value, "nodamage" ) == 0 ) 
				outInfo->surfaceParms |= SURFPARM_NO_DMG;	
			else if ( strcmp( value, "nolightmap" ) == 0 ) 
				outInfo->surfaceParms |= SURFPARM_NO_LIGHTMAP;
			else if ( strcmp( value, "nonsolid" ) == 0 ) 
				outInfo->surfaceParms |= SURFPARM_NON_SOLID;
			else if ( strcmp( value, "nomarks" ) == 0 ) 
				outInfo->surfaceParms |= SURFPARM_NO_MARKS;
			else if ( strcmp( value, "trans" ) == 0 ) 
				outInfo->surfaceParms |= SURFPARM_TRANS;
		}
		else if ( strcmp( token, "polygonoffset" ) == 0 )
		{
			outInfo->hasPolygonOffset = true;	
		}
		// No globals detected, so we must be a stage.
		else if ( outInfo->stageCount < SHADER_MAX_NUM_STAGES )
		{
			if ( strcmp( token, "clampmap" ) == 0 )
			{
				buffer = ReadToken( outInfo->stageBuffer[ outInfo->stageCount ].mapArg, buffer );
				outInfo->stageBuffer[ outInfo->stageCount ].mapCmd = MAP_CMD_CLAMPMAP;
			}
			else if ( strcmp( token, "map" ) == 0 )
			{
				buffer = ReadToken( outInfo->stageBuffer[ outInfo->stageCount ].mapArg, buffer );
				outInfo->stageBuffer[ outInfo->stageCount ].mapCmd = MAP_CMD_MAP;
			}
			else if ( strcmp( token, "blendFunc" ) == 0 )
			{
				char value[ SHADER_MAX_TOKEN_CHAR_LENGTH ] = {};
				buffer = ReadToken( value, buffer );

				outInfo->stageBuffer[ outInfo->stageCount ].blendSrc = GL_EnumFromStr( value );
			
				memset( value, 0, sizeof( value ) );
				buffer = ReadToken( value, buffer );

				outInfo->stageBuffer[ outInfo->stageCount ].blendDest = GL_EnumFromStr( value ); 
			}
			else if ( strcmp( token, "rgbGen" ) == 0 )
			{
				char value[ SHADER_MAX_TOKEN_CHAR_LENGTH ] = {};
				buffer = ReadToken( value, buffer );
				
				if ( strcmp( value, "Vertex" ) == 0 )
					outInfo->stageBuffer[ outInfo->stageCount ].rgbGen = RGBGEN_VERTEX;
			}
		}	

		if ( outInfo->stageCount >= SHADER_MAX_NUM_STAGES )
			__nop();

		buffer = ParseEntry( outInfo, map, buffer + 1, level );
	}

	return buffer;
}

// Lol...
static uint8_t IsStubbedStage( const shaderStage_t* stage )
{
	static const size_t STUB_OFFSETS[] = 
	{
		offsetof( shaderStage_t, rgbGen ), sizeof( rgbGen_t ),
		offsetof( shaderStage_t, blendSrc ), sizeof( GLenum ),
		offsetof( shaderStage_t, blendDest ), sizeof( GLenum )
	};

	static const size_t NUM_STUB_OFFSETS = SIGNED_LEN( STUB_OFFSETS );

	const uint8_t* bytes = ( const uint8_t* ) stage;

	for ( size_t i = 0; i < NUM_STUB_OFFSETS; i += 2 )
	{
		const uint8_t* member = bytes + STUB_OFFSETS[ i ];
		for ( size_t byte = 0; byte < STUB_OFFSETS[ i + 1 ]; ++byte )
			if ( member[ byte ] )
				return 0;
	}

	return 1;
}

static void ParseShader( mapData_t* map, const std::string& filepath )
{
	FILE* file = fopen( filepath.c_str(), "r" );
	MLOG_ASSERT( file, "Could not open file \'%s\'", filepath.c_str() );

	fseek( file, 0, SEEK_END );
	size_t charlen = ftell( file );
	char* fileBuffer = new char[ charlen ]();
	fseek( file, 0, SEEK_SET );
	fread( fileBuffer, 1, charlen, file );
	fclose( file );

	const char* pChar = fileBuffer;

	std::vector< shaderInfo_t > entries;
	while ( *pChar )
	{
		pChar = SkipInvalid( pChar );
		
		shaderInfo_t entry = {};
		pChar = ParseEntry( &entry, map, pChar, 0 );
		
		// Look for stages which are effectively "stubs" and therefore need to use default render parameters
		for ( int i = 0; i < entry.stageCount; ++i )
			entry.stageBuffer[ i ].isStub = IsStubbedStage( entry.stageBuffer + i );

		entries.push_back( entry );
	}
}

void LoadShaders( mapData_t* map )
{
	std::string shaderRootDir( map->basePath );
	shaderRootDir.append( "scripts/" );

	// Find shader files
	//char filePattern[ MAX_PATH ];
	WIN32_FIND_DATAA findFileData;
	
	HANDLE file;

	file = FindFirstFileA( ( shaderRootDir + "*" ).c_str(), &findFileData ); 

	int success = file != INVALID_HANDLE_VALUE;

	while ( success )
	{
		success = FindNextFileA( file, &findFileData );

		std::string ext;
		if ( FileGetExt( ext, std::string( findFileData.cFileName ) ) )
		{
			if ( ext == "shader" )
			{
				ParseShader( map, shaderRootDir + std::string( findFileData.cFileName ) );
			}
		}
	}
}