#include "effect_shader.h"
#include "q3bsp.h"
#include "log.h"

#define SHADER_MAX_NUM_STAGES 3 
#define SHADER_MAX_TOKEN_CHAR_LENGTH 64

// Info can be obtained from http://toolz.nexuizninjaz.com/shader/

enum surfaceParms_t
{
	SURFPARM_ALPHA_SHADOW		= 1 << 1,
	SURFPARM_AREA_PORTAL		= 1 << 2,
	SURFPARM_CLUSTER_PORTAL		= 1 << 3,
	SURFPARM_DO_NOT_ENTER		= 1 << 4,
	SURFPARM_FLESH				= 1 << 5,
	SURFPARM_FOG				= 1 << 6,
	SURFPARM_LAVA				= 1 << 7,
	SURFPARM_METAL_STEPS		= 1 << 8,
	SURFPARM_NO_DMG				= 1 << 9,
	SURFPARM_NO_DLIGHT			= 1 << 10,
	SURFPARM_NO_DRAW			= 1 << 11,
	SURFPARM_NO_DROP			= 1 << 12,
	SURFPARM_NO_IMPACT			= 1 << 13,
	SURFPARM_NO_MARKS			= 1 << 14,
	SURFPARM_NO_LIGHTMAP		= 1 << 15,
	SURFPARM_NO_STEPS			= 1 << 16,
	SURFPARM_NON_SOLID			= 1 << 17,
	SURFPARM_ORIGIN				= 1 << 18,
	SURFPARM_PLAYER_CLIP		= 1 << 19,
	SURFPARM_SLICK				= 1 << 20,
	SURFPARM_SLIME				= 1 << 21,
	SURFPARM_STRUCTURAL			= 1 << 22,
	SURFPARM_TRANS				= 1 << 23,
	SURFPARM_WATER				= 1 << 24
};	

enum rgbGen_t
{
	RGBGEN_IDENTITY_LIGHTING = 1,
	RGBGEN_IDENTITY,
	RGBGEN_ENTITY,
	RGBGEN_ONE_MINUS_ENTITY,
	RGBGEN_VERTEX,
	RGBGEN_ONE_MINUS_VERTEX,
	RGBGEN_LIGHTING_DIFFUSE,
	RGBGEN_WAVE
};

enum tokType_t
{
	TOKTYPE_VALID = 0,
	TOKTYPE_GENERIC, // newlines, indents, whitespace, etc.
	TOKTYPE_COMMENT
};

struct shaderStage_t
{
	GLenum blendSrc;
	GLenum blendDest;
	
	rgbGen_t rgbGen;
	
	char clampmap[ SHADER_MAX_TOKEN_CHAR_LENGTH ];
};

struct shaderInfo_t
{
	char name[ SHADER_MAX_TOKEN_CHAR_LENGTH ];
	char qerEditorImage[ SHADER_MAX_TOKEN_CHAR_LENGTH ];
	char hasPolygonOffset;

	uint32_t surfaceParms;
	uint32_t stageCount;

	shaderStage_t stageBuffer[ SHADER_MAX_NUM_STAGES ];
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
		if ( strcmp( token, "qer_editorimage" ) == 0 )
		{
			buffer = ReadToken( outInfo->qerEditorImage, buffer );
		}
		else if ( strcmp( token, "surfaceparm" ) == 0 )
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
				buffer = ReadToken( outInfo->stageBuffer[ outInfo->stageCount ].clampmap, buffer );
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

		buffer = ParseEntry( outInfo, map, buffer + 1, level );
	}

	return buffer;
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

	while ( *pChar )
	{
		pChar = SkipInvalid( pChar );
		
		shaderInfo_t entry = {};
		pChar = ParseEntry( &entry, map, pChar, 0 );
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