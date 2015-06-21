#include "effect_shader.h"
#include "q3bsp.h"
#include "io.h"
#include "shader.h"
#include "glutil.h"
#include <sstream>

shaderStage_t::shaderStage_t( void )
	: texTransform( 1.0f ),
	  program( nullptr )
{
	isDepthPass = FALSE;
	hasTexMod = FALSE;

	textureSlot = 0;

	rgbSrc = GL_ONE;
	rgbDest = GL_ZERO;

	alphaSrc = GL_ONE;
	alphaDest = GL_ZERO;

	depthFunc = GL_LEQUAL;

	alphaGen = 0.0f;

	rgbGen = RGBGEN_IDENTITY;
	alphaFunc = ALPHA_FUNC_UNDEFINED;
	mapCmd = MAP_CMD_UNDEFINED;
	mapType = MAP_TYPE_UNDEFINED;

	memset( texturePath, 0, sizeof( texturePath ) );
}

shaderInfo_t::shaderInfo_t( void )
{
	memset( name, 0, sizeof( name ) );

	hasLightmap = FALSE;
	hasPolygonOffset = FALSE;
	
	deformCmd = VERTEXDEFORM_CMD_UNDEFINED;
	deformFn = VERTEXDEFORM_FUNC_UNDEFINED;
	
	surfaceParms = 0;
	loadFlags = 0;
	stageCount = 0;
	
	surfaceLight = 0.0f;
	tessSize = 0.0f;
}

enum tokType_t
{
	TOKTYPE_VALID = 0,
	TOKTYPE_GENERIC, // newlines, indents, whitespace, etc.
	TOKTYPE_COMMENT
};

static int gLineCount = 0;

static INLINE GLenum GL_EnumFromStr( const char* str )
{
	if ( strcmp( str, "gl_src_alpha" ) == 0 ) return GL_SRC_ALPHA;
	
	if ( strcmp( str, "gl_one_minus_src_alpha" ) == 0 ) return GL_ONE_MINUS_SRC_ALPHA;
	if ( strcmp( str, "gl_one_minus_src_color" ) == 0 ) return GL_ONE_MINUS_SRC_COLOR;

	if ( strcmp( str, "gl_dst_color" ) == 0 ) return GL_DST_COLOR;
	if ( strcmp( str, "gl_src_color" ) == 0 ) return GL_SRC_COLOR;

	if ( strcmp( str, "gl_zero" ) == 0 ) return GL_ZERO;
	if ( strcmp( str, "gl_one" ) == 0 ) return GL_ONE;

	return 0;
}

static INLINE GLenum GL_DepthFuncFromStr( const char* str )
{
	if ( strcmp( str, "equal" ) == 0 )
		return GL_EQUAL;

	return GL_LEQUAL;
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
		*pOut++ = tolower( *buffer++ );
		charCount++;
	}

	callCount++;

	return buffer;
}

static float ReadFloat( const char*& buffer )
{
	char f[ 12 ] = {};
	buffer = ReadToken( f, buffer );
	return ( float ) strtod( f, NULL );
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
			{
				outInfo->surfaceParms |= SURFPARM_NO_DMG;	
			}
			else if ( strcmp( value, "nolightmap" ) == 0 ) 
			{
				outInfo->surfaceParms |= SURFPARM_NO_LIGHTMAP;
			}
			else if ( strcmp( value, "nonsolid" ) == 0 ) 
			{
				outInfo->surfaceParms |= SURFPARM_NON_SOLID;
			}
			else if ( strcmp( value, "nomarks" ) == 0 )
			{
				outInfo->surfaceParms |= SURFPARM_NO_MARKS;
			}
			else if ( strcmp( value, "trans" ) == 0 ) 
			{
				outInfo->surfaceParms |= SURFPARM_TRANS;
			}
		}
		else if ( strcmp( token, "tesssize" ) == 0 || strcmp( token, "q3map_tesssize" ) == 0 )
		{
			outInfo->tessSize = ReadFloat( buffer );
		}
		else if ( strcmp( token, "deformvertexes" ) == 0 )
		{
			char value[ SHADER_MAX_TOKEN_CHAR_LENGTH ] = {};
			buffer = ReadToken( value, buffer );

			if ( strcmp( value, "wave" ) == 0 )
				outInfo->deformCmd = VERTEXDEFORM_CMD_WAVE;
			else if ( strcmp( value, "normal" ) == 0 )
				outInfo->deformCmd = VERTEXDEFORM_CMD_NORMAL;
			else if ( strcmp( value, "bulge" ) == 0 )
				outInfo->deformCmd = VERTEXDEFORM_CMD_BULGE;

			// Bulge and normal/wave signatures differ significantly, so we separate paths here
			switch ( outInfo->deformCmd )
			{
			case VERTEXDEFORM_CMD_NORMAL:
			case VERTEXDEFORM_CMD_WAVE:
				outInfo->deformParms.spread = ReadFloat( buffer ); 
			
				memset( value, 0, sizeof( value ) );
				buffer = ReadToken( value, buffer );

				if ( strcmp( value, "triangle" ) == 0 )
					outInfo->deformFn = VERTEXDEFORM_FUNC_TRIANGLE;
				else if ( strcmp( value, "sin" ) == 0 )
					outInfo->deformFn = VERTEXDEFORM_FUNC_SIN;
				else if ( strcmp( value, "square" ) == 0 )
					outInfo->deformFn = VERTEXDEFORM_FUNC_SQUARE;
				else if ( strcmp( value, "sawtooth" ) == 0 )
					outInfo->deformFn = VERTEXDEFORM_FUNC_SAWTOOTH;
				else if ( strcmp( value, "inversesawtooth" ) == 0 )
					outInfo->deformFn = VERTEXDEFORM_FUNC_INV_SAWTOOTH;

				outInfo->deformParms.base = ReadFloat( buffer );
				outInfo->deformParms.amplitude = ReadFloat( buffer );
				
				// Normal command has no phase translation
				if ( outInfo->deformCmd == VERTEXDEFORM_CMD_WAVE )
					outInfo->deformParms.phase = ReadFloat( buffer );
				
				outInfo->deformParms.frequency = ReadFloat( buffer );

				break;
            default:
                MLOG_WARNING( "Unsupported vertex deform found!" );
				break;
			}
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
		else if ( strcmp( token, "nopicmip" ) == 0 )
		{
			outInfo->loadFlags ^= Q3LOAD_TEXTURE_MIPMAP;
		}
		// No globals detected, so we must be a stage.
		else if ( outInfo->stageCount < SHADER_MAX_NUM_STAGES )
		{
			if ( strcmp( token, "clampmap" ) == 0 )
			{
				buffer = ReadToken( outInfo->stageBuffer[ outInfo->stageCount ].texturePath, buffer );
				outInfo->stageBuffer[ outInfo->stageCount ].mapCmd = MAP_CMD_CLAMPMAP;
			}
			else if ( strcmp( token, "map" ) == 0 )
			{
				buffer = ReadToken( outInfo->stageBuffer[ outInfo->stageCount ].texturePath, buffer );
				outInfo->stageBuffer[ outInfo->stageCount ].mapCmd = MAP_CMD_MAP;

				assert( strcmp( outInfo->stageBuffer[ outInfo->stageCount ].texturePath, "$whiteimage" ) != 0 && "No support for white image yet" );

				if ( strcmp( outInfo->stageBuffer[ outInfo->stageCount ].texturePath, "$lightmap" ) == 0 )
				{
					outInfo->hasLightmap = TRUE;
					outInfo->stageBuffer[ outInfo->stageCount ].mapType = MAP_TYPE_LIGHT_MAP;
				}
				else
				{
					outInfo->stageBuffer[ outInfo->stageCount ].mapType = MAP_TYPE_IMAGE;
				}
			}
			else if ( strcmp( token, "blendfunc" ) == 0 )
			{
				char value[ SHADER_MAX_TOKEN_CHAR_LENGTH ] = {};
				buffer = ReadToken( value, buffer );

				if ( strcmp( value, "add" ) == 0 )
				{
					outInfo->stageBuffer[ outInfo->stageCount ].rgbSrc = GL_ONE;
					outInfo->stageBuffer[ outInfo->stageCount ].rgbDest = GL_ONE;
				}
				else if ( strcmp( value, "blend" ) == 0 )
				{
					outInfo->stageBuffer[ outInfo->stageCount ].rgbSrc = GL_SRC_ALPHA;
					outInfo->stageBuffer[ outInfo->stageCount ].rgbDest = GL_ONE_MINUS_SRC_ALPHA;
				}
				else if ( strcmp( value, "filter" ) == 0 )
				{
					outInfo->stageBuffer[ outInfo->stageCount ].rgbSrc = GL_DST_COLOR;
					outInfo->stageBuffer[ outInfo->stageCount ].rgbDest = GL_ZERO;
				}
				else
				{
					outInfo->stageBuffer[ outInfo->stageCount ].rgbSrc = GL_EnumFromStr( value );
				
					memset( value, 0, sizeof( value ) );
					buffer = ReadToken( value, buffer );

					outInfo->stageBuffer[ outInfo->stageCount ].rgbDest = GL_EnumFromStr( value ); 
				}

				
				if ( outInfo->surfaceParms & SURFPARM_TRANS )
				{
					outInfo->stageBuffer[ outInfo->stageCount ].alphaSrc = GL_SRC_ALPHA;
					outInfo->stageBuffer[ outInfo->stageCount ].alphaDest = GL_ONE_MINUS_SRC_ALPHA;
				}
				else
				{
					outInfo->stageBuffer[ outInfo->stageCount ].alphaSrc = 
						outInfo->stageBuffer[ outInfo->stageCount ].rgbSrc;
					outInfo->stageBuffer[ outInfo->stageCount ].alphaDest =
						outInfo->stageBuffer[ outInfo->stageCount ].rgbDest;
				}
			}
			else if ( strcmp( token, "alphafunc" ) == 0 )
			{
				char value[ SHADER_MAX_TOKEN_CHAR_LENGTH ] = {};
				buffer = ReadToken( value, buffer );
				
				if ( strcmp( value, "ge128" ) == 0 )
					outInfo->stageBuffer[ outInfo->stageCount ].alphaFunc = ALPHA_FUNC_GEQUAL_128;
				else if ( strcmp( value, "gT0" ) == 0 )
					outInfo->stageBuffer[ outInfo->stageCount ].alphaFunc = ALPHA_FUNC_GTHAN_0;
				else if ( strcmp( value, "lt128" ) == 0 )
					outInfo->stageBuffer[ outInfo->stageCount ].alphaFunc = ALPHA_FUNC_LTHAN_128;

			}
			else if ( strcmp( token, "rgbgen" ) == 0 )
			{
				char value[ SHADER_MAX_TOKEN_CHAR_LENGTH ] = {};
				buffer = ReadToken( value, buffer );
					
				if ( strcmp( value, "vertex" ) == 0 )
					outInfo->stageBuffer[ outInfo->stageCount ].rgbGen = RGBGEN_VERTEX;
				else if ( strcmp( value, "identity" ) == 0 )
					outInfo->stageBuffer[ outInfo->stageCount ].rgbGen = RGBGEN_IDENTITY;
				else if ( strcmp( value, "identitylighting" ) == 0 )
					outInfo->stageBuffer[ outInfo->stageCount ].rgbGen = RGBGEN_IDENTITY_LIGHTING;
			}
			else if ( strcmp( token, "tcmod" ) == 0 )
			{
				char type[ SHADER_MAX_TOKEN_CHAR_LENGTH ] = {};
				buffer = ReadToken( type, buffer );

				outInfo->stageBuffer[ outInfo->stageCount ].hasTexMod = TRUE;

				if ( strcmp( type, "scale" ) == 0 )
				{
					float s = ReadFloat( buffer );
					float t = ReadFloat( buffer );

					//outInfo->stageBuffer[ outInfo->stageCount ].texTransformStack.push( glm::mat2( 1.0f, 1.0f, 1.0f, 1.0f ) );

					outInfo->stageBuffer[ outInfo->stageCount ].texTransformStack.push( glm::mat2( glm::vec2( s, s ), glm::vec2( t, t ) ) );
				}
				else if ( strcmp( type, "turb" ) == 0 )
				{
					effect_t op;

					op.name = "tcModTurb";
					op.type	= EFFECT_WAVE;
	
					op.data.wave.base = ReadFloat( buffer );
					op.data.wave.amplitude = ReadFloat( buffer );
					op.data.wave.phase = ReadFloat( buffer );
					op.data.wave.frequency = ReadFloat( buffer );

					outInfo->stageBuffer[ outInfo->stageCount ].effects.push_back( op );
				}
				else if ( strcmp( type, "scroll" ) == 0 )
				{
					effect_t op;

					op.name = "tcModScroll";
					op.type = EFFECT_VEC4;

					op.data.xyzw[ 0 ] = ReadFloat( buffer );
					op.data.xyzw[ 1 ] = ReadFloat( buffer );

					outInfo->stageBuffer[ outInfo->stageCount ].effects.push_back( op );
				}
			}
			else if ( strcmp( token, "depthfunc" ) == 0 )
			{
				char value[ SHADER_MAX_TOKEN_CHAR_LENGTH ] = {};
				buffer = ReadToken( value, buffer );
				outInfo->stageBuffer[ outInfo->stageCount ].depthFunc = GL_DepthFuncFromStr( value );
			}
			else if ( strcmp( token, "depthwrite" ) == 0 )
			{
				outInfo->stageBuffer[ outInfo->stageCount ].isDepthPass = TRUE;
			}
		}	

		if ( outInfo->stageCount >= SHADER_MAX_NUM_STAGES )
			__nop();

		buffer = ParseEntry( outInfo, buffer + 1, level );
	}

	return buffer;
}

static void ParseShader( shaderMap_t& entries, uint32_t loadFlags, const std::string& filepath )
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

		entry.loadFlags = loadFlags;
		pChar = ParseEntry( &entry, pChar, 0 );

		// Perform post processing.
		// We check for stubs (i.e., stages with features not taken into consideration yet),
		// and right multiply any texture transforms, since the order each transform is specified
		// in the effect shader is the order it needs to be transformed in ( and because we're using OpenGL ).
		for ( int i = 0; i < entry.stageCount; ++i )
		{
			while ( entry.stageBuffer[ i ].texTransformStack.size() > 0 )
			{
				entry.stageBuffer[ i ].texTransform *= entry.stageBuffer[ i ].texTransformStack.top();
				entry.stageBuffer[ i ].texTransformStack.pop();
			}
		}
		entries.insert( shaderMapEntry_t( entry.name, entry ) );
	}

	gLineCount = 0;
}

static void GenShaderPrograms( shaderMap_t& effectShaders )
{
	// Print the generated shaders to a text file
	FILE* f = fopen( "log/shader_gen.txt", "w" );

	auto LWriteTexture = []( std::vector< std::string >& fragmentSrc, 
		const shaderStage_t& stage, bool doGammaCorrect, const char* discardPredicate ) 
	{
		if ( stage.alphaGen != 0.0f )
		{
			fragmentSrc.push_back( "\tconst float alphaGen = " + std::to_string( stage.alphaGen ) + std::to_string( ';' ) );
			fragmentSrc.push_back( 
				"\tvec4 color = vec4( texture( sampler0, st ).rgb, alphaGen ) * vec4( frag_Color.rgb, alphaGen );" );
		}
		else
		{
			fragmentSrc.push_back( "\tvec4 color = texture( sampler0, st ) * frag_Color;" );
		}

		if ( discardPredicate )
		{
			fragmentSrc.push_back( "\tif ( " + std::string( discardPredicate ) + " )" );
			fragmentSrc.push_back( "\t{" );
			fragmentSrc.push_back( "\t\tdiscard;" );
			fragmentSrc.push_back( "\t}" );
		}

		if ( doGammaCorrect )
		{
			fragmentSrc.insert( fragmentSrc.end(), 
			{
				"\tcolor.r = pow( color.r, gamma );",
				"\tcolor.g = pow( color.g, gamma );",
				"\tcolor.b = pow( color.b, gamma );"
			} );
		}
		
		fragmentSrc.push_back( "\tfragment = color;" );
	};

	auto LJoinLines = []( const std::vector< std::string >& lines ) -> std::string
	{
		std::stringstream shaderSrc;

		for ( const std::string& line: lines )
		{
			shaderSrc << line << '\n';
		}

		// Append the end bracket for the main function
		shaderSrc << '}';

		return shaderSrc.str();
	};

	// Convert each effect stage to its GLSL equivalent
	for ( auto& entry: effectShaders )
	{
		shaderInfo_t& shader = entry.second;
		fprintf( f, "------------------------------------------\n%s\n", shader.name );

		for ( int j = 0; j < shader.stageCount; ++j )
		{	
			// Uniform variable names
			std::vector< std::string > uniforms = { "sampler0" };
			std::vector< std::string > attribs = { "position", "color", "tex0" }; 

			// Load vertex header;
			std::vector< std::string > vertexSrc = 
			{	
				"#version 450", 
				"layout( location = 0 ) in vec3 position;", 
				"layout( location = 1 ) in vec4 color;", 
				"layout( location = 2 ) in vec2 tex0;",
				"layout( std140 ) uniform Transforms {",
				"\tmat4 viewToClip;",
				"\tmat4 modelToView;",
				"};",
				"out vec2 frag_Tex;",
				"out vec4 frag_Color;",
				"void main(void) {",
				"\tgl_Position = viewToClip * modelToView * vec4( position, 1.0 );"
			};

			vertexSrc.push_back( "\tfrag_Tex = tex0;" );
			
			if ( Shade_IsIdentColor( shader.stageBuffer[ j ] ) )
			{ 
				vertexSrc.push_back( "\tfrag_Color = vec4(1.0f);" );
			}
			else
			{
				vertexSrc.push_back( "\tfrag_Color = color;" );
			}

			// Load fragment header;
			// Unspecified alphaGen implies a default 1.0 alpha channel
			std::vector< std::string > fragmentSrc =
			{
				"#version 450",
				"in vec2 frag_Tex;",
				"in vec4 frag_Color;",
				"const float gamma = 1.0 / 2.2;",
				"uniform sampler2D sampler0;",
				"out vec4 fragment;",
				"void main(void) {"
			};

			const size_t fragUnifOffset = 3;

			// If we have a matrix transform for the texcoord, go with that first.
			if ( shader.stageBuffer[ j ].hasTexMod )
			{
				fragmentSrc.insert( fragmentSrc.begin() + fragUnifOffset, "uniform mat2 texTransform;" );
				fragmentSrc.push_back( "\tvec2 st = texTransform * frag_Tex;" );
				uniforms.push_back( "texTransform" );
			}
			else
			{
				fragmentSrc.push_back( "\tvec2 st = frag_Tex;" );
			}

			for ( const effect_t& op: shader.stageBuffer[ j ].effects )
			{
				// Modify the texture coordinate as necessary before we write to the texture
				if ( op.name == "tcModTurb" )
				{
					fragmentSrc.insert( fragmentSrc.begin() + fragUnifOffset, "uniform float tcModTurb;" );
					fragmentSrc.push_back( "\tst += tcModTurb;" );
					uniforms.push_back( "tcModTurb" );
				}
				else if ( op.name == "tcModScroll" )
				{
					fragmentSrc.insert( fragmentSrc.begin() + fragUnifOffset, "uniform vec4 tcModScroll;" );
					fragmentSrc.push_back( "\tst += tcModScroll.xy * tcModScroll.zw;" );
					uniforms.push_back( "tcModScroll" );
				}
			}

			// We assess whether or not we need to add conservative depth to aid in OpenGL optimization,
			// given the potential for fragment discard if an alpha function is defined
			if ( shader.stageBuffer[ j ].alphaFunc != ALPHA_FUNC_UNDEFINED )
			{
				// Enable extensions directly below the version decl to aid in readability
				fragmentSrc.insert( fragmentSrc.begin() + 1, "#extension GL_ARB_conservative_depth: enable" );
				fragmentSrc.push_back( "layout( depth_unchanged ) float gl_FragDepth;" );
			}

			switch ( shader.stageBuffer[ j ].alphaFunc )
			{
			case ALPHA_FUNC_UNDEFINED:
				LWriteTexture( fragmentSrc, shader.stageBuffer[ j ], true, NULL );
				break;
			case ALPHA_FUNC_GEQUAL_128:
				LWriteTexture( fragmentSrc, shader.stageBuffer[ j ], true, "color.a < 0.5" );
				break;
			case ALPHA_FUNC_GTHAN_0:
				LWriteTexture( fragmentSrc, shader.stageBuffer[ j ], true, "color.a == 0" );
				break;
			case ALPHA_FUNC_LTHAN_128:
				LWriteTexture( fragmentSrc, shader.stageBuffer[ j ], true, "color.a >= 0.5" );
				break;
			}

			const std::string& vertexString = LJoinLines( vertexSrc );
			const std::string& fragmentString = LJoinLines( fragmentSrc );

			shader.stageBuffer[ j ].program = std::make_shared< Program >( vertexString, fragmentString, uniforms, attribs );
			
			fprintf( f, "[ %i ] [\n\n Vertex \n\n%s \n\n Fragment \n\n%s \n\n ]\n\n", 
				j, 
				vertexString.c_str(), 
				fragmentString.c_str() );
		}
	}

	fclose( f );
}

static void GenShaderTextures( const mapData_t* map, shaderMap_t& effectShaders )
{
	for ( auto& entry: effectShaders )
	{
		shaderInfo_t& shader = entry.second;

		for ( int i = 0; i < shader.stageCount; ++i )
		{
			shaderStage_t& stage = shader.stageBuffer[ i ];

			stage.textureSlot = i;
			stage.texture.wrap = stage.mapCmd == MAP_CMD_CLAMPMAP ? GL_CLAMP_TO_EDGE : GL_REPEAT;
			stage.texture.mipmap = !!( shader.loadFlags & Q3LOAD_TEXTURE_MIPMAP );

			if ( stage.mapType == MAP_TYPE_IMAGE )
			{
				std::string texFileRoot( map->basePath );
				texFileRoot.append( stage.texturePath );

				bool success = stage.texture.LoadFromFile( texFileRoot.c_str(), shader.loadFlags );

				if ( success )
				{
					for ( effect_t& op: stage.effects )
					{
						if ( op.name == "tcModScroll" )
						{
							op.data.xyzw[ 2 ] = ( float ) stage.texture.width;
							op.data.xyzw[ 3 ] = ( float ) stage.texture.height;
						}
					}
				}
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

	file = FindFirstFileA( ( shaderRootDir + "*.shader" ).c_str(), &findFileData ); 
	int success = file != INVALID_HANDLE_VALUE;

	while ( success )
	{
		ParseShader( effectShaders, loadFlags, shaderRootDir + std::string( findFileData.cFileName ) );
		success = FindNextFileA( file, &findFileData );
	}
	
	GenShaderPrograms( effectShaders );
	GenShaderTextures( map, effectShaders );
}
