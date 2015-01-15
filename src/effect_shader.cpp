#include "effect_shader.h"
#include "q3bsp.h"
#include "log.h"
#include "shader.h"
#include "glutil.h"
#include <sstream>

enum tokType_t
{
	TOKTYPE_VALID = 0,
	TOKTYPE_GENERIC, // newlines, indents, whitespace, etc.
	TOKTYPE_COMMENT
};

static int gLineCount = 0;

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
	static const char syms[] = 
	{
		'\t', ' ', '\n', '\r',
		'*', '[', ']', '(', ')'
	};

	if ( *c == '\n' )
		gLineCount++;

	// If we have an indent, space, newline, or a comment, then the token is invalid
	if ( *c == '/' && *( c + 1 ) == '/' )
		return TOKTYPE_COMMENT;

	for ( int i = 0; i < SIGNED_LEN( syms ); ++i )
		if ( *c == syms[ i ] )
			return TOKTYPE_GENERIC;

	return TOKTYPE_VALID;
}

static INLINE const char* NextLine( const char* buffer )
{
	while ( *buffer != '\n' )
		buffer++;

	//gLineCount++;

	// Add one to skip the newline
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
	static int callCount = 0;

	buffer = SkipInvalid( buffer );

	// Parse token
	int charCount = 0;
	char* pOut = out;
	while ( Token( buffer ) == TOKTYPE_VALID )
	{
		*pOut++ = *buffer++;
		charCount++;
	}

	callCount++;

	return buffer;
}

// Returns the char count to increment the filebuffer by
static const char* ParseEntry( shaderInfo_t* outInfo, const char* buffer, const int level )
{
	if ( !( *( buffer = SkipInvalid( buffer ) ) ) )
	{
		return buffer;
	}

	// Begin stage?
	if ( *buffer == '{' )
		return ParseEntry( outInfo, buffer + 1, level + 1 );

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
			return ParseEntry( outInfo, buffer + 1, level - 1 );
		}
	}

	// No invalid tokens ( e.g., spaces, indents, comments, etc. ); so, this must be a header
	if ( level == 0 )
	{
		// Find our entity string
		//char entCandidate[ SHADER_MAX_TOKEN_CHAR_LENGTH ];
		//memset( entCandidate, 0, sizeof( entCandidate ) );

		buffer = ReadToken( outInfo->name, buffer );		

		buffer = ParseEntry( outInfo, buffer, level );
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
		else if ( strcmp( token, "q3map_surfacelight" ) == 0 )
		{
			char value[ SHADER_MAX_TOKEN_CHAR_LENGTH ] = {};
			buffer = ReadToken( value, buffer );
			outInfo->surfaceLight = ( float )strtod( value, NULL );
		}
		else if ( strcmp( token, "polygonoffset" ) == 0 )
		{
			outInfo->hasPolygonOffset = TRUE;	
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

				if ( strcmp( outInfo->stageBuffer[ outInfo->stageCount ].mapArg, "$lightmap" ) == 0 )
					outInfo->hasLightmap = TRUE;
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

		buffer = ParseEntry( outInfo, buffer + 1, level );
	}

	return buffer;
}

static uint8_t IsStubbedStage( shaderStage_t* stage )
{
	// Last condition is for the rgbGen vars which are currently supported.
	return ( stage->mapCmd == 0 
		  || stage->blendSrc == 0 
		  || stage->blendDest == 0 
		  || stage->rgbGen == 0 
		  || strcmp( stage->mapArg, "$whiteimage" ) == 0
		  || ( stage->rgbGen != RGBGEN_VERTEX && stage->rgbGen != RGBGEN_ONE_MINUS_VERTEX && stage->rgbGen != RGBGEN_IDENTITY && stage->rgbGen != RGBGEN_IDENTITY_LIGHTING ) );  
}

static void ParseShader( shaderMap_t& entries, const std::string& filepath )
{
	FILE* file = fopen( filepath.c_str(), "rb" );
	MLOG_ASSERT( file, "Could not open file \'%s\'", filepath.c_str() );

	fseek( file, 0, SEEK_END );
	size_t charlen = ftell( file );
	char* fileBuffer = new char[ charlen + 1 ]();
	fileBuffer[ charlen ] = '\0';
	fseek( file, 0, SEEK_SET );
	fread( fileBuffer, 1, charlen, file );
	fclose( file );

	const char* pChar = fileBuffer;
	const char* end = fileBuffer + charlen;

	while ( *pChar )
	{	
		shaderInfo_t entry;
		memset( &entry, 0, sizeof( entry ) - sizeof( entry.stageBuffer ) );

		for ( int i = 0; i < SHADER_MAX_NUM_STAGES; ++i )
		{
			memset( &entry.stageBuffer[ i ], 0, sizeof( entry.stageBuffer[ i ] ) - sizeof( entry.stageBuffer[ i ].uniforms ) );
		}

		pChar = ParseEntry( &entry, pChar, 0 );
		
		// Look for stages which are effectively "stubs" and therefore need to use default render parameters
		for ( int i = 0; i < entry.stageCount; ++i )
		{	
			if ( IsStubbedStage( entry.stageBuffer + i ) )
			{
				// Define some defaults for lightmaps and remove their stub constraint
				if ( strcmp( entry.stageBuffer[ i ].mapArg, "$lightmap" ) == 0 )
				{
					entry.stageBuffer[ i ].blendSrc = GL_SRC_ALPHA;
					entry.stageBuffer[ i ].blendDest = GL_ONE_MINUS_SRC_ALPHA;
				}
				else
				{
					entry.stageStubCount += 1;
					entry.stageBuffer[ i ].isStub = TRUE;
				}
			}
		}

		entries.insert( shaderMapEntry_t( entry.name, entry ) );
	}

	gLineCount = 0;
}

static void GenShaderPrograms( shaderMap_t& effectShaders )
{
	// Base variables are required for all programs.
	const char* baseVertex = "#version 420\n"
							 "layout( location = 0 ) in vec3 position;\n"
							 "layout( location = 1 ) in vec2 tex0;\n"
							 "uniform mat4 modelToView;\n"
							 "uniform mat4 viewToClip;\n"
							 "const float gamma = 1.0 / 2.2;\n";
							 

	const char* baseFragment =	"#version 420\n"
								"in vec2 frag_Tex;\n" 
								"in vec4 frag_Color;\n"
								"uniform sampler2D sampler0;\n"
								"out vec4 fragment;\n";

	// Convert each effect stage to its GLSL equivalent
	for ( auto& entry: effectShaders )
	{
		shaderInfo_t& shader = entry.second;

		for ( int j = 0; j < shader.stageCount; ++j )
		{
			if ( shader.stageBuffer[ j ].isStub )
				continue;

			// These are the bodies of the main function in the programs
			std::stringstream fragcmp, vertcmp;
				
			// Entire program source strings
			std::stringstream vertexSrc, fragmentSrc;

			std::vector< std::string > uniformStrings = { "modelToView", "viewToClip", "sampler0" };

			// Load vertex header;
			vertexSrc << baseVertex;
			if ( shader.stageBuffer[ j ].rgbGen == RGBGEN_IDENTITY || shader.stageBuffer[ j ].rgbGen == RGBGEN_IDENTITY_LIGHTING )
				vertexSrc << "const vec4 color = vec4( 1.0 );\n";
			else
				vertexSrc << "layout( location = 2 ) in vec4 color;\n";

			vertexSrc << "out vec2 frag_Tex;\n"
						 "out vec4 frag_Color;\n";

			// Load vertex shader body
			vertcmp <<	"gl_Position = viewToClip * modelToView * vec4( position, 1.0 );\n"
						"frag_Tex = tex0;\n"
						"frag_Color = pow( color, vec4( gamma ) );\n";

			// Load fragment header;
			// Unspecified alphaGen implies a default 1.0 alpha channel
			fragmentSrc << baseFragment;
			if ( shader.stageBuffer[ j ].alphaGen == 0.0f )
				fragmentSrc << "const float alphaGen = 1.0;\n";
			else
				fragmentSrc << "const float alphaGen = " << shader.stageBuffer[ j ].alphaGen << ";\n";

			// And fragment shader body...
			const std::string& rgbCmp = "texture( sampler0, frag_Tex ).rgb * frag_Color.rgb";
			switch ( shader.stageBuffer[ j ].rgbGen )
			{
			case RGBGEN_ONE_MINUS_VERTEX:
				fragcmp << "fragment = vec4( vec3( 1.0 ) - " << rgbCmp << ", alphaGen );\n";
				break;
			default:
				fragcmp << "fragment = vec4( " << rgbCmp << ", alphaGen );\n";
				break;
			}

			vertexSrc << "void main() { \n" << vertcmp.str() << " }\n";
			fragmentSrc << "void main() { \n" << fragcmp.str() << " }\n";

			GLuint shaders[] = 
			{
				CompileShaderSource( vertexSrc.str().c_str(), GL_VERTEX_SHADER ),
				CompileShaderSource( fragmentSrc.str().c_str(), GL_FRAGMENT_SHADER )
			};

			shader.stageBuffer[ j ].programID = LinkProgram( shaders, 2 );

			for ( uint32_t u = 0; u < uniformStrings.size(); ++u )
			{
				GLint uniform;
				GL_CHECK( uniform = glGetUniformLocation( shader.stageBuffer[ j ].programID, uniformStrings[ u ].c_str() ) );
				shader.stageBuffer[ j ].uniforms.insert( glHandleMapEntry_t( uniformStrings[ u ], uniform ) );
			}
		}
	}
}

static void GenShaderTextures( const mapData_t* map, uint32_t loadFlags, shaderMap_t& effectShaders )
{
	for ( auto& entry: effectShaders )
	{
		shaderInfo_t& shader = entry.second;

		// HACK: search for a face in our map data with a corresponding lightmap index.
		// This is a very, very temporary solution.
		int lightmapIndex = -1;
		if ( shader.hasLightmap )
		{
			for ( int i = 0; i < map->numFaces; ++i )
			{
				if ( map->faces[ i ].texture != -1 && strcmp( map->textures[ map->faces[ i ].texture ].name, shader.name ) == 0 )
				{
					lightmapIndex = map->faces[ i ].lightmapIndex;
					break;
				}
			}
		}

		for ( int i = 0; i < shader.stageCount; ++i )
		{
			if ( shader.stageBuffer[ i ].isStub )
				continue;

			shader.stageBuffer[ i ].texOffset = i;

			GL_CHECK( glGenTextures( 1, &shader.stageBuffer[ i ].textureObj ) );
			GL_CHECK( glGenSamplers( 1, &shader.stageBuffer[ i ].samplerObj ) );

			GL_CHECK( glSamplerParameteri( shader.stageBuffer[ i ].samplerObj, GL_TEXTURE_MAG_FILTER, GL_LINEAR ) );
			GL_CHECK( glSamplerParameteri( shader.stageBuffer[ i ].samplerObj, GL_TEXTURE_MIN_FILTER, GL_LINEAR ) );

			std::string texFileRoot( map->basePath );

			// LoadTextureFromFile already binds the texture for us, so for $lightmap it's actually necessary
			if ( strcmp( shader.stageBuffer[ i ].mapArg, "$lightmap" ) == 0 )
			{
				shader.stageBuffer[ i ].mapType = MAP_TYPE_LIGHT_MAP;
				
				GL_CHECK( glBindTexture( GL_TEXTURE_2D, shader.stageBuffer[ i ].textureObj ) );
				GL_CHECK( glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB8, BSP_LIGHTMAP_WIDTH, BSP_LIGHTMAP_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, map->lightmaps[ lightmapIndex ].map ) );
				GL_CHECK( glBindTexture( GL_TEXTURE_2D, 0 ) );

				GL_CHECK( glSamplerParameteri( shader.stageBuffer[ i ].samplerObj, GL_TEXTURE_WRAP_S, GL_REPEAT ) );  
				GL_CHECK( glSamplerParameteri( shader.stageBuffer[ i ].samplerObj, GL_TEXTURE_WRAP_T, GL_REPEAT ) ); 
			}
			else
			{
				shader.stageBuffer[ i ].mapType = MAP_TYPE_IMAGE;

				texFileRoot.append( shader.stageBuffer[ i ].mapArg );

				assert( LoadTextureFromFile( 
					texFileRoot.c_str(), 
					shader.stageBuffer[ i ].textureObj, 
					shader.stageBuffer[ i ].samplerObj, 
					loadFlags,
					shader.stageBuffer[ i ].mapCmd == MAP_CMD_CLAMPMAP ? GL_CLAMP_TO_EDGE : GL_REPEAT ) 
				);
			}
		}
	}
}

void LoadShaders( const mapData_t* map, uint32_t loadFlags, shaderMap_t& effectShaders )
{
	std::string shaderRootDir( map->basePath );
	shaderRootDir.append( "scripts/" );

	// Find shader files
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
				ParseShader( effectShaders, shaderRootDir + std::string( findFileData.cFileName ) );
			}
		}
	}
	
	GenShaderPrograms( effectShaders );
	GenShaderTextures( map, loadFlags, effectShaders );
}