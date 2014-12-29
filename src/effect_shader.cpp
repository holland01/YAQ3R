#include "effect_shader.h"
#include "q3bsp.h"
#include "log.h"

#define MAX_TOKEN_CHAR_LENGTH 64

// See: http://icculus.org/gtkradiant/documentation/Q3AShader_Manual/ch03/pg3_1.htm for context
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

struct shaderInfo_t
{
	char name[ MAX_TOKEN_CHAR_LENGTH ];
	char qerEditorImage[ MAX_TOKEN_CHAR_LENGTH ];

	uint32_t surfaceParms;
};

static INLINE bool IsToken( const char* c )
{
	// If we have an indent, space, newline, or a comment, then the token is invalid
	return !( ( *c == '\t' || *c == ' ' || *c == '\n' ) || ( *c == '/' && *( c + 1 ) == '/' ) );
}

static const char* ReadToken( char* out, const char* buffer )
{
	while ( !IsToken( buffer ) ) 
		*buffer++;

	int charCount = 0;

	char* pOut = out;
	while ( IsToken( buffer ) )
	{
		*pOut++ = *buffer++;
		charCount++;
	}

	return buffer;  
}

// Returns the char count to increment the filebuffer by
static const char* ParseEntry( shaderInfo_t* outInfo, mapData_t* map, const char* buffer, int level )
{
	// Begin stage?
	if ( *buffer == '{' )
		return ParseEntry( outInfo, map, buffer + 1, level + 1 );

	// End stage; we done
	if ( *buffer == '}' )
		return buffer + 1;

	// No invalid tokens ( e.g., spaces, indents, comments, etc. ); so, this must be a header
	if ( level == 0 )
	{
		// Find our entity string
		char entCandidate[ MAX_TOKEN_CHAR_LENGTH ];
		memset( entCandidate, 0, sizeof( entCandidate ) );

		buffer = ReadToken( entCandidate, buffer );

		for ( int i = 0; i < map->numTextures; ++i )
		{
			if ( strcmp( entCandidate, map->textures[ i ].name ) == 0 )
			{
				memcpy( outInfo->name, map->textures[ i ].name, sizeof( entCandidate ) );
				break;
			}
		}

		buffer = ParseEntry( outInfo, map, buffer + 1, level );
	}
	else
	{
		char token[ MAX_TOKEN_CHAR_LENGTH ];
		memset( token, 0, sizeof( token ) );

		buffer = ReadToken( token, buffer );

		if ( strcmp( token, "qer_editorimage" ) == 0 )
		{
			buffer = ReadToken( outInfo->qerEditorImage, buffer );
		}
		else if ( strcmp( token, "surfaceparm" ) == 0 )
		{
			char value[ MAX_TOKEN_CHAR_LENGTH ];
			memset( value, 0, sizeof( value ) ); 

			buffer = ReadToken( value, buffer );
			
			if ( strcmp( value, "nodamage" ) ) 
				outInfo->surfaceParms |= SURFPARM_NO_DMG;	
			else if ( strcmp( value, "nolightmap" ) ) 
				outInfo->surfaceParms |= SURFPARM_NO_LIGHTMAP;
			else if ( strcmp( value, "nonsolid" ) ) 
				outInfo->surfaceParms |= SURFPARM_NON_SOLID;
			else if ( strcmp( value, "nomarks" ) ) 
				outInfo->surfaceParms |= SURFPARM_NO_MARKS;
			else if ( strcmp( value, "trans" ) ) 
				outInfo->surfaceParms |= SURFPARM_TRANS;
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
		if ( !IsToken( pChar ) )
		{
			pChar++;
			continue;
		}
		
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