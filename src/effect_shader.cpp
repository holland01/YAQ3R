#include "effect_shader.h"
#include "q3bsp.h"
#include "io.h"
#include "shader.h"
#include "glutil.h"
#include <sstream>

static INLINE GLsizei GL_EnumFromStr( const char* str );
static INLINE GLsizei GL_DepthFuncFromStr( const char* str );
static float ReadFloat( const char*& buffer );
static const char* ReadToken( char* out, const char* buffer );

using stageEvalFunc_t = std::function< bool( const char* & buffer, shaderInfo_t* outInfo, char* token ) >;

#define STAGE_READ_FUNC []( const char* & buffer, shaderInfo_t* outInfo, char* token ) -> bool

#define ZEROTOK( t ) ( memset( t, 0, sizeof( char ) * SHADER_MAX_TOKEN_CHAR_LENGTH ) )


std::map< std::string, stageEvalFunc_t > stageReadFuncs = 
{
	{
		"surfaceparm",
		STAGE_READ_FUNC
		{
			ZEROTOK( token );
			buffer = ReadToken( token, buffer );
				
			if ( strcmp( token, "nodamage" ) == 0 ) 
			{
				outInfo->surfaceParms |= SURFPARM_NO_DMG;	
			}
			else if ( strcmp( token, "nolightmap" ) == 0 ) 
			{
				outInfo->surfaceParms |= SURFPARM_NO_LIGHTMAP;
			}
			else if ( strcmp( token, "nonsolid" ) == 0 ) 
			{
				outInfo->surfaceParms |= SURFPARM_NON_SOLID;
			}
			else if ( strcmp( token, "nomarks" ) == 0 )
			{
				outInfo->surfaceParms |= SURFPARM_NO_MARKS;
			}
			else if ( strcmp( token, "trans" ) == 0 ) 
			{
				outInfo->surfaceParms |= SURFPARM_TRANS;
			}
			else
			{
				return false;
			}

			return true;
		}
	},
	{
		"deformvertexes",
		STAGE_READ_FUNC
		{
			ZEROTOK( token );
			buffer = ReadToken( token, buffer );

			if ( strcmp( token, "wave" ) == 0 )
			{
				outInfo->deformCmd = VERTEXDEFORM_CMD_WAVE;
			}
			else if ( strcmp( token, "normal" ) == 0 )
			{
				outInfo->deformCmd = VERTEXDEFORM_CMD_NORMAL;
			}
			else if ( strcmp( token, "bulge" ) == 0 )
			{
				outInfo->deformCmd = VERTEXDEFORM_CMD_BULGE;
			}
			else
			{
				return false;
			}

			// Bulge and normal/wave signatures differ significantly, so we separate paths here
			switch ( outInfo->deformCmd )
			{
			case VERTEXDEFORM_CMD_WAVE:
				outInfo->deformParms.spread = ReadFloat( buffer ); 
			
				ZEROTOK( token );
				buffer = ReadToken( token, buffer );

				if ( strcmp( token, "triangle" ) == 0 )
				{
					outInfo->deformFn = VERTEXDEFORM_FUNC_TRIANGLE;
				}
				else if ( strcmp( token, "sin" ) == 0 )
				{
					outInfo->deformFn = VERTEXDEFORM_FUNC_SIN;
				}
				else if ( strcmp( token, "square" ) == 0 )
				{
					outInfo->deformFn = VERTEXDEFORM_FUNC_SQUARE;
				}
				else if ( strcmp( token, "sawtooth" ) == 0 )
				{
					outInfo->deformFn = VERTEXDEFORM_FUNC_SAWTOOTH;
				}
				else if ( strcmp( token, "inversesawtooth" ) == 0 )
				{
					outInfo->deformFn = VERTEXDEFORM_FUNC_INV_SAWTOOTH;
				}

				outInfo->deformParms.base = ReadFloat( buffer );
				outInfo->deformParms.amplitude = ReadFloat( buffer );
				
				// Normal command has no phase translation
				if ( outInfo->deformCmd == VERTEXDEFORM_CMD_WAVE )
				{
					outInfo->deformParms.phase = ReadFloat( buffer );
				}

				outInfo->deformParms.frequency = ReadFloat( buffer );

				outInfo->deform = true;
				break;

			default:
				MLOG_WARNING_SANS_FUNCNAME( "deformvertexes", "Unsupported vertex deform found!" );
				outInfo->deform = false;
				return false;
				break;
			}

			return true;
		}
	},
	{
		"nopicmip",
		STAGE_READ_FUNC
		{
			outInfo->loadFlags ^= Q3LOAD_TEXTURE_MIPMAP;
			return true;
		}
	},
	{
		"tesssize",
		STAGE_READ_FUNC
		{
			outInfo->tessSize = ReadFloat( buffer );
			return true;
		}
	},
	{
		"q3map_tesssize",
		STAGE_READ_FUNC
		{
			outInfo->tessSize = ReadFloat( buffer );
			return true;
		}
	},
	{
		"clampmap",
		STAGE_READ_FUNC
		{
			buffer = ReadToken( outInfo->stageBuffer[ outInfo->stageCount ].texturePath, buffer );
			outInfo->stageBuffer[ outInfo->stageCount ].mapCmd = MAP_CMD_CLAMPMAP;
			outInfo->stageBuffer[ outInfo->stageCount ].mapType = MAP_TYPE_IMAGE;
			return true;
		}
	},
	{
		"map",
		STAGE_READ_FUNC
		{
			buffer = ReadToken( outInfo->stageBuffer[ outInfo->stageCount ].texturePath, buffer );
			outInfo->stageBuffer[ outInfo->stageCount ].mapCmd = MAP_CMD_MAP;

			// TODO: add support for this
			if ( strcmp( outInfo->stageBuffer[ outInfo->stageCount ].texturePath, "$whiteimage" ) == 0 )
			{
				return true;
			}

			if ( strcmp( outInfo->stageBuffer[ outInfo->stageCount ].texturePath, "$lightmap" ) == 0 )
			{
				outInfo->stageBuffer[ outInfo->stageCount ].mapType = MAP_TYPE_LIGHT_MAP;
			}
			else
			{
				outInfo->stageBuffer[ outInfo->stageCount ].mapType = MAP_TYPE_IMAGE;
			}

			return true;
		}
	},
	{
		"blendfunc",
		STAGE_READ_FUNC
		{
			ZEROTOK( token );
			buffer = ReadToken( token, buffer );

			if ( strcmp( token, "add" ) == 0 )
			{
				outInfo->stageBuffer[ outInfo->stageCount ].rgbSrc = GL_ONE;
				outInfo->stageBuffer[ outInfo->stageCount ].rgbDest = GL_ONE;
			}
			else if ( strcmp( token, "blend" ) == 0 )
			{
				outInfo->stageBuffer[ outInfo->stageCount ].rgbSrc = GL_SRC_ALPHA;
				outInfo->stageBuffer[ outInfo->stageCount ].rgbDest = GL_ONE_MINUS_SRC_ALPHA;
			}
			else if ( strcmp( token, "filter" ) == 0 )
			{
				outInfo->stageBuffer[ outInfo->stageCount ].rgbSrc = GL_DST_COLOR;
				outInfo->stageBuffer[ outInfo->stageCount ].rgbDest = GL_ZERO;
			}
			else
			{
				GLsizei blendFactor = GL_EnumFromStr( token );
				if ( blendFactor == -1 )
				{
					return false;
				}

				outInfo->stageBuffer[ outInfo->stageCount ].rgbSrc = ( GLenum ) blendFactor;
						
				ZEROTOK( token );
				buffer = ReadToken( token, buffer );

				blendFactor = GL_EnumFromStr( token );
				if ( blendFactor == -1 )
				{
					outInfo->stageBuffer[ outInfo->stageCount ].rgbDest = outInfo->stageBuffer[ outInfo->stageCount ].rgbSrc;
					return false;
				}

				outInfo->stageBuffer[ outInfo->stageCount ].rgbDest = ( GLenum ) blendFactor;
			}

			return true;
		}
	},
	{
		"alphafunc",
		STAGE_READ_FUNC
		{
			ZEROTOK( token );
			buffer = ReadToken( token, buffer );
				
			if ( strcmp( token, "ge128" ) == 0 )
			{
				outInfo->stageBuffer[ outInfo->stageCount ].alphaFunc = ALPHA_FUNC_GEQUAL_128;
			}
			else if ( strcmp( token, "gT0" ) == 0 )
			{
				outInfo->stageBuffer[ outInfo->stageCount ].alphaFunc = ALPHA_FUNC_GTHAN_0;
			}
			else if ( strcmp( token, "lt128" ) == 0 )
			{
				outInfo->stageBuffer[ outInfo->stageCount ].alphaFunc = ALPHA_FUNC_LTHAN_128;
			}
			else
			{
				return false;
			}

			return true;
		}
	},
	{
		"rgbgen",
		STAGE_READ_FUNC
		{
			ZEROTOK( token );
			buffer = ReadToken( token, buffer );
					
			if ( strcmp( token, "vertex" ) == 0 )
			{
				outInfo->stageBuffer[ outInfo->stageCount ].rgbGen = RGBGEN_VERTEX;
			}
			else if ( strcmp( token, "identity" ) == 0 )
			{
				outInfo->stageBuffer[ outInfo->stageCount ].rgbGen = RGBGEN_IDENTITY;
			}
			else if ( strcmp( token, "identitylighting" ) == 0 )
			{
				outInfo->stageBuffer[ outInfo->stageCount ].rgbGen = RGBGEN_IDENTITY_LIGHTING;
			}
			else
			{
				return false;
			}

			return true;
		}
	},
	{
		"tcgen",
		STAGE_READ_FUNC
		{
			ZEROTOK( token );
			buffer = ReadToken( token, buffer );

			if ( strcmp( token, "environment" ) == 0 )
			{
				outInfo->stageBuffer[ outInfo->stageCount ].tcgen = TCGEN_ENVIRONMENT;
			}
			else if ( strcmp( token, "base" ) == 0 )
			{
				outInfo->stageBuffer[ outInfo->stageCount ].tcgen = TCGEN_BASE;
			}
			else if ( strcmp( token, "lightmap" ) == 0 )
			{
				outInfo->stageBuffer[ outInfo->stageCount ].tcgen = TCGEN_LIGHTMAP;
			}
			else
			{
				return false;
			}

			return true;
		} 
	},
	{
		"tcmod",
		STAGE_READ_FUNC
		{
			ZEROTOK( token );
			buffer = ReadToken( token, buffer );

			if ( strcmp( token, "scale" ) == 0 )
			{
				effect_t op;

				op.name = "tcModScale";

				float s = ReadFloat( buffer );
				float t = ReadFloat( buffer );

				op.data.scale2D[ 0 ][ 0 ] = s;
				op.data.scale2D[ 0 ][ 1 ] = s;
				op.data.scale2D[ 1 ][ 0 ] = t;
				op.data.scale2D[ 1 ][ 1 ] = t;

				outInfo->stageBuffer[ outInfo->stageCount ].effects.push_back( op );
			}
			else if ( strcmp( token, "turb" ) == 0 )
			{
				effect_t op;

				op.name = "tcModTurb";
	
				op.data.wave.base = ReadFloat( buffer );
				op.data.wave.amplitude = ReadFloat( buffer );
				op.data.wave.phase = ReadFloat( buffer );
				op.data.wave.frequency = ReadFloat( buffer );

				outInfo->stageBuffer[ outInfo->stageCount ].effects.push_back( op );
			}
			else if ( strcmp( token, "scroll" ) == 0 )
			{
				effect_t op;

				op.name = "tcModScroll";

				op.data.xyzw[ 0 ] = ReadFloat( buffer );
				op.data.xyzw[ 1 ] = ReadFloat( buffer );

				outInfo->stageBuffer[ outInfo->stageCount ].effects.push_back( op );
			}
			else if ( strcmp( token, "rotate" ) == 0 )
			{
				effect_t op;

				op.name = "tcModRotate";					
					
				float angRad = glm::radians( ReadFloat( buffer ) );

				op.data.rotation2D.transform[ 0 ][ 0 ] = glm::cos( angRad );
				op.data.rotation2D.transform[ 0 ][ 1 ] =-glm::sin( angRad );
				op.data.rotation2D.transform[ 1 ][ 0 ] = glm::sin( angRad );
				op.data.rotation2D.transform[ 1 ][ 1 ] = glm::cos( angRad );

				outInfo->stageBuffer[ outInfo->stageCount ].effects.push_back( op );
			}
			else
			{
				return false;
			}

			return true;
		}
	},
	{
		"depthfunc",
		STAGE_READ_FUNC
		{
			ZEROTOK( token );
			buffer = ReadToken( token, buffer );

			GLsizei depthf = GL_DepthFuncFromStr( token );

			if ( depthf == -1 )
			{
				return false;
			}

			outInfo->stageBuffer[ outInfo->stageCount ].depthFunc = ( GLenum ) depthf;
			return true;
		}
	},
	{
		"depthwrite",
		STAGE_READ_FUNC
		{
			outInfo->stageBuffer[ outInfo->stageCount ].depthPass = true;
			return true;
		}
	}
};

shaderStage_t::shaderStage_t( void )
	: tcgen( TCGEN_BASE ),
	  textureSlot( 0 ),
	  rgbSrc( GL_ONE ),
	  rgbDest( GL_ZERO ),
	  alphaSrc( GL_ONE ),
	  alphaDest( GL_ZERO ),
	  depthFunc( GL_LEQUAL ),
	  rgbGen( RGBGEN_IDENTITY_LIGHTING ),
	  alphaFunc( ALPHA_FUNC_UNDEFINED ),
	  mapCmd( MAP_CMD_UNDEFINED ),
	  mapType( MAP_TYPE_UNDEFINED ),
	  alphaGen( 0.0f ),
	  program( nullptr )
{
	memset( texturePath, 0, sizeof( texturePath ) );
}

shaderInfo_t::shaderInfo_t( void )
	:	deform( false ),
		deformCmd( VERTEXDEFORM_CMD_UNDEFINED ),
		deformFn( VERTEXDEFORM_FUNC_UNDEFINED ),
		surfaceParms( 0 ),
		loadFlags( 0 ),
		stageCount( 0 ),
		surfaceLight( 0.0f ),
		tessSize( 0.0f )
	  
{
	memset( name, 0, sizeof( name ) );
}

enum tokType_t
{
	TOKTYPE_VALID = 0,
	TOKTYPE_GENERIC, // newlines, indents, whitespace, etc.
	TOKTYPE_COMMENT
};

static int gLineCount = 0;

static INLINE GLsizei GL_EnumFromStr( const char* str )
{
	// blending
	if ( strcmp( str, "gl_one_minus_src_alpha" ) == 0 ) return GL_ONE_MINUS_SRC_ALPHA;
	if ( strcmp( str, "gl_one_minus_src_color" ) == 0 ) return GL_ONE_MINUS_SRC_COLOR;
	if ( strcmp( str, "gl_one_minus_dst_alpha" ) == 0 ) return GL_ONE_MINUS_DST_ALPHA;

	if ( strcmp( str, "gl_dst_color" ) == 0 ) return GL_DST_COLOR;
	if ( strcmp( str, "gl_src_color" ) == 0 ) return GL_SRC_COLOR;
	if ( strcmp( str, "gl_src_alpha" ) == 0 ) return GL_SRC_ALPHA;

	if ( strcmp( str, "gl_zero" ) == 0 ) return GL_ZERO;
	if ( strcmp( str, "gl_one" ) == 0 ) return GL_ONE;

	// depth funcs
	if ( strcmp( str, "gl_never" ) == 0 ) return GL_NEVER;
	if ( strcmp( str, "gl_less" ) == 0 ) return GL_LESS;
	if ( strcmp( str, "gl_equal" ) == 0 ) return GL_EQUAL;
	if ( strcmp( str, "gl_lequal" ) == 0 ) return GL_LEQUAL;
	if ( strcmp( str, "gl_greater" ) == 0 ) return GL_GREATER;
	if ( strcmp( str, "gl_notequal" ) == 0 ) return GL_NOTEQUAL;
	if ( strcmp( str, "gl_gequal" ) == 0 ) return GL_GEQUAL;
	if ( strcmp( str, "gl_always" ) == 0 ) return GL_ALWAYS;

	return -1;
}

static INLINE GLsizei GL_DepthFuncFromStr( const char* str )
{
	if ( strcmp( str, "equal" ) == 0 ) return GL_EQUAL;
	if ( strcmp( str, "lequal" ) == 0 ) return GL_LEQUAL;

	// The manual seems to insinuate that gl_ prefixes won't be used for depth functions. However, this is used just in case...
	return GL_EnumFromStr( str );
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
		if ( !*buffer )
		{
			break;
		}

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
static const char* ParseEntry( shaderInfo_t* outInfo, const char* buffer, int level )
{
	char token[ 64 ];
	
	char first = 0; 

	while ( true )
	{
		memset( token, 0, sizeof( token ) );

		if ( !*( buffer = ReadToken( token, buffer ) ) )
		{
			break;
		}

		first = *buffer;

evaluate_tok:
		// Begin stage?
		if ( *token == '{' )
		{
			level += 1;
			continue;
		}

		// End stage; we done
		if ( *token == '}' )
		{
			// We're back out into the main level, so we're finished with this entry.
			if ( level == 1 )
			{
				break;
			}
			// We're not in the main level, but we're leaving this stage, so decrease our level by 1 and add on to our stageCount
			else
			{
				outInfo->stageCount += 1;
				level -= 1;
				continue;	
			}
		}

		// No invalid tokens ( e.g., spaces, indents, comments, etc. ); so, this must be a header
		if ( level == 0 )
		{
			strcpy( outInfo->name, token );
			continue;
		}

		const std::string strToken( token );
		if ( stageReadFuncs.find( strToken ) == stageReadFuncs.end() )
		{
			continue;
		}

		if ( !stageReadFuncs.at( strToken )( buffer, outInfo, token ) )
		{
			goto evaluate_tok;
		}
	}

	if ( *buffer == first )
	{
		buffer++;
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

	auto LJoinLines = []( shaderStage_t& stage, std::vector< std::string >& lines, std::vector< std::string >& attribs ) -> std::string
	{
		std::stringstream shaderSrc;

		for ( std::string& line: lines )
		{
			size_t index = line.find_first_of( "$" );
			if ( index != std::string::npos )
			{
				if ( stage.mapType == MAP_TYPE_LIGHT_MAP )
				{
					line.replace( index, 5, "lightmap" );
					attribs[ 2 ] = "lightmap";
				}
				else
				{
					line.erase( index, 1 );
				}
			}

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
				"layout( location = 2 ) in vec2 $tex0;",
				"layout( std140 ) uniform Transforms {",
				"\tmat4 viewToClip;",
				"\tmat4 modelToView;",
				"};",
				"out vec2 frag_Tex;",
				"out vec4 frag_Color;",
				"void main(void) {",
				"\tgl_Position = viewToClip * modelToView * vec4( position, 1.0 );"
			};

			vertexSrc.push_back( "\tfrag_Tex = $tex0;" );
			
			if ( Shader_StageHasIdentityColor( shader.stageBuffer[ j ] ) )
			{ 
				vertexSrc.push_back( "\tfrag_Color = vec4( 1.0 );" );
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

			fragmentSrc.push_back( "\tvec2 st = frag_Tex;" );

			for ( const effect_t& op: shader.stageBuffer[ j ].effects )
			{
				// Modify the texture coordinate as necessary before we write to the texture
				if ( op.name == "tcModTurb" )
				{
					fragmentSrc.insert( fragmentSrc.begin() + fragUnifOffset, "uniform float tcModTurb;" );
					fragmentSrc.push_back( "\tst *= tcModTurb;" );
					uniforms.push_back( "tcModTurb" );
				}
				else if ( op.name == "tcModScroll" )
				{
					fragmentSrc.insert( fragmentSrc.begin() + fragUnifOffset, "uniform vec4 tcModScroll;" );
					fragmentSrc.push_back( "\tst += tcModScroll.xy /** tcModScroll.zw*/;" );
					fragmentSrc.push_back( "\tst = mod( st, 1.0 );" );
					uniforms.push_back( "tcModScroll" );
				}
				else if ( op.name == "tcModRotate" )
				{
					fragmentSrc.insert( fragmentSrc.begin() + fragUnifOffset, "uniform mat2 texRotate;" );
					fragmentSrc.insert( fragmentSrc.begin() + fragUnifOffset, "uniform vec2 texCenter;" );
					fragmentSrc.push_back( "\tst += texRotate * ( frag_Tex - texCenter );" );

					uniforms.push_back( "texRotate" );
					uniforms.push_back( "texCenter" );
				}
				else if ( op.name == "tcModScale" )
				{
					fragmentSrc.insert( fragmentSrc.begin() + fragUnifOffset, "uniform mat2 tcModScale;" );
					fragmentSrc.push_back( "\tst = tcModScale * st;" );
					uniforms.push_back( "tcModScale" );
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

			const std::string& vertexString = LJoinLines( shader.stageBuffer[ j ], vertexSrc, attribs );
			const std::string& fragmentString = LJoinLines( shader.stageBuffer[ j ], fragmentSrc, attribs );

			shader.stageBuffer[ j ].program = std::make_shared< Program >( vertexString, fragmentString, uniforms, attribs );
			
			fprintf( f, "[ %i ] [\n\n Vertex \n\n%s \n\n Fragment \n\n%s \n\n ]\n\n", 
				j, 
				vertexString.c_str(), 
				fragmentString.c_str() );
		}
	}

	fclose( f );
}

static void LoadStageTexture( shaderInfo_t& info, int i, const mapData_t* map )
{
	shaderStage_t& stage = info.stageBuffer[ i ];

	if ( stage.mapType == MAP_TYPE_IMAGE )
	{
		stage.textureSlot = i;
		stage.texture.wrap = stage.mapCmd == MAP_CMD_CLAMPMAP ? GL_CLAMP_TO_EDGE : GL_REPEAT;
		stage.texture.mipmap = !!( info.loadFlags & Q3LOAD_TEXTURE_MIPMAP );

		std::string texFileRoot( map->basePath );
		texFileRoot.append( stage.texturePath );

		if ( !stage.texture.LoadFromFile( texFileRoot.c_str(), info.loadFlags ) )
		{
			MLOG_WARNING( "Could not load texture file \"%s\"", texFileRoot.c_str() );
		}
	}
}

void Shader_SetEffectTextureData( effect_t& op, const texture_t& texture )
{
	if ( op.name == "tcModScroll" )
	{
		op.data.xyzw[ 2 ] = ( float ) texture.width;
		op.data.xyzw[ 3 ] = ( float ) texture.height;
	}
	else if ( op.name == "tcModRotate" )
	{
		op.data.rotation2D.center[ 0 ] = 
			( ( float ) texture.width * 0.5f ) / ( float ) texture.width;
		op.data.rotation2D.center[ 1 ] = 
			( ( float ) texture.height * 0.5f ) / ( float ) texture.height;
	}
}

void Shader_LoadAll( const mapData_t* map, shaderMap_t& effectShaders, uint32_t loadFlags )
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
	
	for ( auto& entry: effectShaders )
	{
		for ( int i = 0; i < entry.second.stageCount; ++i )
		{
			LoadStageTexture( entry.second, i, map );
		}
	}
}
