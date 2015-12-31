#include "effect_shader.h"
#include "q3bsp.h"
#include "io.h"
#include "shader.h"
#include "glutil.h"
#include "renderer/texture.h"
#include <sstream>

static INLINE GLsizei GL_EnumFromStr( const char* str );
static INLINE GLsizei GL_DepthFuncFromStr( const char* str );
static float ReadFloat( const char*& buffer );
static const char* ReadToken( char* out, const char* buffer );

using stageEvalFunc_t = std::function< bool( const char* & buffer, shaderInfo_t* outInfo, shaderStage_t& theStage, char* token ) >;

#define STAGE_READ_FUNC []( const char* & buffer, shaderInfo_t* outInfo, shaderStage_t& theStage, char* token ) -> bool

#define ZEROTOK( t ) ( memset( t, 0, sizeof( char ) * SHADER_MAX_TOKEN_CHAR_LENGTH ) );

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
                outInfo->deformParms.data.wave.spread = ReadFloat( buffer );
			
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

                outInfo->deformParms.data.wave.base = ReadFloat( buffer );
                outInfo->deformParms.data.wave.amplitude = ReadFloat( buffer );
				
				// Normal command has no phase translation
				if ( outInfo->deformCmd == VERTEXDEFORM_CMD_WAVE )
				{
                    outInfo->deformParms.data.wave.phase = ReadFloat( buffer );
				}

                outInfo->deformParms.data.wave.frequency = ReadFloat( buffer );

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
		"cull",
		STAGE_READ_FUNC
		{
			ZEROTOK( token );
			buffer = ReadToken( token, buffer );

			if ( strcmp( token, "front" ) == 0 )
			{
				outInfo->cullFace = GL_FRONT;
			}
			else if ( strcmp( token, "back" ) == 0 )
			{
				outInfo->cullFace = GL_BACK;
			}
			else if ( strcmp( token, "none" ) == 0 || strcmp( token, "disable" ) == 0 )
			{
				// shaderInfo_t::cullFace is GL_FALSE (i.e., no cull) by default, so no worries...
				return true;
			}
			else
			{
				return false;
			}

			return true;
		}
	},
	{
		"nopicmip",
		STAGE_READ_FUNC
		{
            outInfo->localLoadFlags ^= Q3LOAD_TEXTURE_MIPMAP;
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
            buffer = ReadToken( &theStage.texturePath[ 0 ], buffer );
            theStage.mapCmd = MAP_CMD_CLAMPMAP;
            theStage.mapType = MAP_TYPE_IMAGE;
			return true;
		}
	},
	{
		"map",
		STAGE_READ_FUNC
		{
            buffer = ReadToken( &theStage.texturePath[ 0 ], buffer );
            theStage.mapCmd = MAP_CMD_MAP;

			// TODO: add support for this
            if ( strcmp( &theStage.texturePath[ 0 ], "$whiteimage" ) == 0 )
			{
				return true;
			}

            if ( strcmp( &theStage.texturePath[ 0 ], "$lightmap" ) == 0 )
			{
                theStage.mapType = MAP_TYPE_LIGHT_MAP;
			}
			else
			{
                theStage.mapType = MAP_TYPE_IMAGE;
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
                theStage.blendSrc = GL_ONE;
                theStage.blendDest = GL_ONE;
			}
			else if ( strcmp( token, "blend" ) == 0 )
			{
                theStage.blendSrc = GL_SRC_ALPHA;
                theStage.blendDest = GL_ONE_MINUS_SRC_ALPHA;
			}
			else if ( strcmp( token, "filter" ) == 0 )
			{
                theStage.blendSrc = GL_DST_COLOR;
                theStage.blendDest = GL_ZERO;
			}
			else
			{
				GLsizei blendFactor = GL_EnumFromStr( token );
				if ( blendFactor == -1 )
				{
					return false;
				}

                theStage.blendSrc = ( GLenum ) blendFactor;
						
				ZEROTOK( token );
				buffer = ReadToken( token, buffer );

				blendFactor = GL_EnumFromStr( token );
				if ( blendFactor == -1 )
				{
                    theStage.blendDest = theStage.blendSrc;
					return false;
				}

                theStage.blendDest = ( GLenum ) blendFactor;
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
                theStage.alphaFunc = ALPHA_FUNC_GEQUAL_128;
			}
			else if ( strcmp( token, "gT0" ) == 0 )
			{
                theStage.alphaFunc = ALPHA_FUNC_GTHAN_0;
			}
			else if ( strcmp( token, "lt128" ) == 0 )
			{
                theStage.alphaFunc = ALPHA_FUNC_LTHAN_128;
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
                theStage.rgbGen = RGBGEN_VERTEX;
			}
			else if ( strcmp( token, "identity" ) == 0 )
			{
                theStage.rgbGen = RGBGEN_IDENTITY;
			}
			else if ( strcmp( token, "identitylighting" ) == 0 )
			{
                theStage.rgbGen = RGBGEN_IDENTITY_LIGHTING;
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
                theStage.tcgen = TCGEN_ENVIRONMENT;
				outInfo->surfaceParms |= SURFPARM_ENVMAP;
			}
			else if ( strcmp( token, "base" ) == 0 )
			{
                theStage.tcgen = TCGEN_BASE;
			}
			else if ( strcmp( token, "lightmap" ) == 0 )
			{
                theStage.tcgen = TCGEN_LIGHTMAP;
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

                /*
                NOTE: a scale may imply a division by the value, versus a multiplication. I'm not sure...

                if ( s != 0.0f )
                    s = 1.0f / s;

                if ( t != 0.0f )
                    t = 1.0f / t;
                */

				op.data.scale2D[ 0 ][ 0 ] = s;
				op.data.scale2D[ 0 ][ 1 ] = s;
				op.data.scale2D[ 1 ][ 0 ] = t;
				op.data.scale2D[ 1 ][ 1 ] = t;

                theStage.effects.push_back( op );
			}
			else if ( strcmp( token, "turb" ) == 0 )
			{
				effect_t op;

				op.name = "tcModTurb";
	
				op.data.wave.base = ReadFloat( buffer );
				op.data.wave.amplitude = ReadFloat( buffer );
				op.data.wave.phase = ReadFloat( buffer );
				op.data.wave.frequency = ReadFloat( buffer );

                theStage.effects.push_back( op );
			}
			else if ( strcmp( token, "scroll" ) == 0 )
			{
				effect_t op;

				op.name = "tcModScroll";

				op.data.xyzw[ 0 ] = ReadFloat( buffer );
				op.data.xyzw[ 1 ] = ReadFloat( buffer );

                theStage.effects.push_back( op );
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

                theStage.effects.push_back( op );
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

            theStage.depthFunc = ( GLenum ) depthf;
			return true;
		}
	},
	{
		"depthwrite",
		STAGE_READ_FUNC
		{
            theStage.depthPass = true;
			return true;
		}
	}
};

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

    shaderStage_t stage;

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
                //outInfo->stageBuffer.insert( outInfo->stageBuffer.begin(), stage );
                outInfo->stageBuffer.push_back( stage );
                stage = shaderStage_t();
				outInfo->stageCount += 1;
				level -= 1;
				continue;	
			}
		}

		// No invalid tokens ( e.g., spaces, indents, comments, etc. ); so, this must be a header
		if ( level == 0 )
		{
            strcpy( &outInfo->name[ 0 ], token );
			continue;
		}

		const std::string strToken( token );
		if ( stageReadFuncs.find( strToken ) == stageReadFuncs.end() )
		{
			continue;
		}

        if ( !stageReadFuncs.at( strToken )( buffer, outInfo, stage, token ) )
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
    std::vector< uint8_t > fileBuffer;
    if ( !File_GetBuf( fileBuffer, filepath ) )
    {
        MLOG_WARNING( "Could not open shader file \'%s\'", filepath.c_str() );
        return;
    }

    const char* pChar = ( const char* ) &fileBuffer[ 0 ];

	while ( *pChar )
	{	
		shaderInfo_t entry;

        entry.localLoadFlags = loadFlags;
		pChar = ParseEntry( &entry, pChar, 0 );

        entries.insert( shaderMapEntry_t( std::string( &entry.name[ 0 ], strlen( &entry.name[ 0 ] ) ), entry ) );
	}

	gLineCount = 0;
}

static inline void AddDiscardIf( std::vector< std::string >& fragmentSrc, const std::string& discardPredicate )
{
	fragmentSrc.push_back( "\tif ( " + discardPredicate + " )" );
	fragmentSrc.push_back( "\t{" );
	fragmentSrc.push_back( "\t\tdiscard;" );
	fragmentSrc.push_back( "\t}" );
}

static inline void WriteTexture( std::vector< std::string >& fragmentSrc, 
        const shaderStage_t& stage, const char* discardPredicate )
{
	std::string sampleTextureExpr;

	if ( stage.mapType == MAP_TYPE_IMAGE ) 
	{
		if ( stage.mapCmd == MAP_CMD_CLAMPMAP )
            fragmentSrc.push_back("\tst = clamp( applyTransform( st ), imageTransform.xy, applyTransform( vec2( 0.99 ) ) );");
		else
            fragmentSrc.push_back("\tst = applyTransform( mod( st, vec2( 0.99 ) ) );");

        sampleTextureExpr = "texture( sampler0, st )";
	}
	else
	{
        sampleTextureExpr = "texture( sampler0, vec3( st * bias.xy, bias.z ) )";
    }

    // Some shader entries will incorporate specific alpha values
	if ( stage.alphaGen != 0.0f )
	{
        fragmentSrc.push_back( "\tconst float alphaGen = " + std::to_string( stage.alphaGen ) + std::to_string( ';' ) );
		fragmentSrc.push_back( "\tvec4 color = vec4( " + sampleTextureExpr + ".rgb, alphaGen ) * vec4( frag_Color.rgb, alphaGen );" );
	}
	else
	{
		fragmentSrc.push_back( "\tvec4 color = " + sampleTextureExpr + " * frag_Color;" );
	}

	//if ( stage.mapCmd == MAP_CMD_CLAMPMAP && stage.alphaFunc != ALPHA_FUNC_GTHAN_0 )
		//AddDiscardIf( fragmentSrc, "color.a == 0.0" ); 

    // Is used occasionally, for example in situations like a bad alpha value.
	if ( discardPredicate )
		AddDiscardIf( fragmentSrc, std::string( discardPredicate ) );

    // Gamma correction
    fragmentSrc.insert( fragmentSrc.end(),
    {
        "\tcolor.r = pow( color.r, gamma );",
        "\tcolor.g = pow( color.g, gamma );",
        "\tcolor.b = pow( color.b, gamma );"
    } );

	fragmentSrc.push_back( "\tfragment = color;" );
}

 // Quick subroutine enabling the calculation of environment map;
// uses a simple form of displacement mapping to achieve desired results.
static void AddCalcEnvMap( std::vector< std::string >& destGLSL,
                               const std::string& vertexID,
                               const std::string& normalID,
                               const std::string& eyeID )
{
	destGLSL.insert( destGLSL.end(),
        {
            "\tvec3 dirToEye = normalize( " + eyeID + " - " + vertexID + " );",
            "\tvec3 R = normalize( 2.0 * " + normalID + " * dot( dirToEye, " + normalID + " ) - dirToEye );",
            "\tvec2 displace = ( R.xz ) * 0.5;",
            "\tvec2 st = vec2( 0.5 );",
            "\tst.s -= displace.x;",
            "\tst.t += displace.y;"
        } );
}

static inline std::string JoinLines( std::vector< std::string >& lines, std::vector< std::string >& attribs )
{
	std::stringstream shaderSrc;

	for ( std::string& line: lines )
		shaderSrc << line << '\n';

	// Append the end bracket for the main function
	shaderSrc << '}';

	return shaderSrc.str();
}

static void GenShaderPrograms( shaderMap_t& effectShaders )
{
	// Print the generated shaders to a text file
	FILE* f = fopen( "log/shader_gen.txt", "w" );

	// Convert each effect stage to its GLSL equivalent
	for ( auto& entry: effectShaders )
	{
		shaderInfo_t& shader = entry.second;
        fprintf( f, "------------------------------------------\n%s\n", &shader.name[ 0 ] );

		for ( int j = 0; j < shader.stageCount; ++j )
		{	
			shaderStage_t& stage = shader.stageBuffer[ j ];

			// Uniform variable names
			std::vector< std::string > uniforms = { "sampler0" };

			if ( stage.mapType == MAP_TYPE_IMAGE )
			{
				uniforms.push_back( "imageTransform" );
				uniforms.push_back( "imageScaleRatio" );
			}
			else
			{
				uniforms.push_back( "bias" );
			}
			
			std::vector< std::string > attribs = { "position", "color", "tex0" }; 

			const std::string texCoordName( ( stage.mapType == MAP_TYPE_LIGHT_MAP )? "lightmap": "tex0" );
            const size_t vertGlobalVarInsertOffset = 4;

			// Load vertex header;
			std::vector< std::string > vertexSrc = 
			{	
				"#version 450", 
				"layout( location = 0 ) in vec3 position;", 
				"layout( location = 1 ) in vec4 color;", 
				"layout( location = 2 ) in vec2 " + texCoordName + ";",
				"out vec2 frag_Tex;",
				"out vec4 frag_Color;",
				"void main(void) {",
			};

            if ( stage.tcgen == TCGEN_ENVIRONMENT )
            {
                vertexSrc.insert( vertexSrc.begin() + vertGlobalVarInsertOffset, "uniform vec3 surfaceNormal;" );

                /*
                vertexSrc.insert( vertexSrc.begin() + vertGlobalVarInsertOffset + 2, "out vec3 frag_WorldNormal;" );
                vertexSrc.insert( vertexSrc.begin() + vertGlobalVarInsertOffset + 2, "out vec3 frag_WorldVertex;" );
                vertexSrc.insert( vertexSrc.begin() + vertGlobalVarInsertOffset + 2, "out vec3 frag_ViewOrigin;" );
                */


                uniforms.push_back( "surfaceNormal" );
            }

            vertexSrc.insert( vertexSrc.begin() + vertGlobalVarInsertOffset,
            {
                "layout( std140 ) uniform Transforms {",
                "\tmat4 viewToClip;",
                "\tmat4 modelToView;",
                "};"
            } );

            vertexSrc.push_back( "\tgl_Position = viewToClip * modelToView * vec4( position, 1.0 );" );

            if ( stage.tcgen == TCGEN_ENVIRONMENT )
            {
                /*
                vertexSrc.insert( vertexSrc.end(),
                {
                    "mat4 T = mat4( 1.0 );",
                    "frag_WorldNormal = mat3( T ) * surfaceNormal;",
                    "frag_WorldVertex = vec3( T * vec4( position, 1.0 ) ); // position is already calculated relative to the world, so there is no need to apply an offset",
                    "frag_ViewOrigin = -vec3( modelToView[ 3 ] );"
                } );

                */

                AddCalcEnvMap( vertexSrc, "position", "surfaceNormal", "vec3( -modelToView[ 3 ] )" );
                vertexSrc.push_back( "frag_Tex = st;" );
            }
            else
            {
                vertexSrc.push_back( "\tfrag_Tex = " + texCoordName + ";" );
            }

            if ( stage.rgbGen == RGBGEN_IDENTITY || stage.rgbGen == RGBGEN_IDENTITY_LIGHTING )
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
				"const float gamma = 1.0 / 3.0;",
				//"uniform sampler2DArray sampler0;",
				//"uniform sampler2D sampler0;",
				//"uniform vec4 imageTransform;",
				//"uniform vec2 imageScaleRatio;",
				//"uniform vec3 bias;",
				"out vec4 fragment;",
				"void main(void) {"
			};

            const size_t fragGlobalDeclOffset = 4;

			if ( stage.mapType == MAP_TYPE_IMAGE )
			{
				std::initializer_list<std::string> data  =
				{
					"uniform sampler2D sampler0;",
					"uniform vec4 imageTransform;",
					"uniform vec2 imageScaleRatio;",
					"vec2 applyTransform(in vec2 coords) {",
                    "\treturn coords * imageTransform.zw * imageScaleRatio + imageTransform.xy;",
					"}"
				};

				fragmentSrc.insert( fragmentSrc.begin() + fragGlobalDeclOffset, data );
			}
			else
			{
				std::initializer_list<std::string> data =
				{
					"uniform sampler2DArray sampler0;",
					"uniform vec3 bias;"
				};

				fragmentSrc.insert( fragmentSrc.begin() + fragGlobalDeclOffset, data );
			}

            /*if ( stage.tcgen == TCGEN_ENVIRONMENT )
            {

                fragmentSrc.insert( fragmentSrc.begin() + 1, "in vec3 frag_WorldNormal;" );
                fragmentSrc.insert( fragmentSrc.begin() + 1, "in vec3 frag_WorldVertex;" );
                fragmentSrc.insert( fragmentSrc.begin() + 1, "in vec3 frag_ViewOrigin;" );

                LAddCalcEnvmap( fragmentSrc, "frag_WorldVertex", "frag_WorldNormal", "frag_ViewOrigin" );

                //fragmentSrc.push_back( "\tvec2 st = frag_Tex;" );

			}
            else
            */
            {
                fragmentSrc.push_back( "\tvec2 st = frag_Tex;" );
            }
			for ( const effect_t& op: shader.stageBuffer[ j ].effects )
			{
				// Modify the texture coordinate as necessary before we write to the texture
				if ( op.name == "tcModTurb" )
				{
                    fragmentSrc.insert( fragmentSrc.begin() + fragGlobalDeclOffset, "uniform float tcModTurb;" );
					fragmentSrc.push_back( "\tst *= tcModTurb;" );
					uniforms.push_back( "tcModTurb" );
				}
				else if ( op.name == "tcModScroll" )
				{
                    fragmentSrc.insert( fragmentSrc.begin() + fragGlobalDeclOffset, "uniform vec4 tcModScroll;" );
					fragmentSrc.push_back( "\tst += tcModScroll.xy * tcModScroll.zw;" );
					fragmentSrc.push_back( "\tst = mod( st, 1.0 );" );
					uniforms.push_back( "tcModScroll" );
				}
				else if ( op.name == "tcModRotate" )
				{
                    fragmentSrc.insert( fragmentSrc.begin() + fragGlobalDeclOffset, "uniform mat2 texRotate;" );
                    fragmentSrc.insert( fragmentSrc.begin() + fragGlobalDeclOffset, "uniform vec2 texCenter;" );
					fragmentSrc.push_back( "\tst += texRotate * ( frag_Tex - texCenter );" );

					uniforms.push_back( "texRotate" );
					uniforms.push_back( "texCenter" );
				}
				else if ( op.name == "tcModScale" )
				{
                    fragmentSrc.insert( fragmentSrc.begin() + fragGlobalDeclOffset, "uniform mat2 tcModScale;" );
					fragmentSrc.push_back( "\tst = tcModScale * st;" );
					uniforms.push_back( "tcModScale" );
				}
			}

			// We assess whether or not we need to add conservative depth to aid in OpenGL optimization,
			// given the potential for fragment discard if an alpha function is defined
			if ( stage.alphaFunc != ALPHA_FUNC_UNDEFINED )
			{
				// Enable extensions directly below the version decl to aid in readability
				fragmentSrc.insert( fragmentSrc.begin() + 1, "#extension GL_ARB_conservative_depth: enable" );
				fragmentSrc.push_back( "layout( depth_unchanged ) float gl_FragDepth;" );
			}

			switch ( stage.alphaFunc )
			{
			case ALPHA_FUNC_UNDEFINED:
                WriteTexture( fragmentSrc, stage, NULL );
				break;
			case ALPHA_FUNC_GEQUAL_128:
                WriteTexture( fragmentSrc, stage, "color.a < 0.5" );
				break;
			case ALPHA_FUNC_GTHAN_0:
                WriteTexture( fragmentSrc, stage, "color.a == 0" );
				break;
			case ALPHA_FUNC_LTHAN_128:
                WriteTexture( fragmentSrc, stage, "color.a >= 0.5" );
				break;
			}

            const std::string& vertexString = JoinLines( vertexSrc, attribs );
            const std::string& fragmentString = JoinLines( fragmentSrc, attribs );

			stage.program = std::make_shared< Program >( vertexString, fragmentString, uniforms, attribs );
			
			fprintf( f, "[ %i ] [\n\n Vertex \n\n%s \n\n Fragment \n\n%s \n\n ]\n\n", 
				j, 
				vertexString.c_str(), 
				fragmentString.c_str() );
		}
	}

	fclose( f );
}

static void LoadStageTexture( glm::ivec2& maxDims, std::vector< gImageParams_t >& images, shaderInfo_t& info, int i, const mapData_t* map )
{
	shaderStage_t& stage = info.stageBuffer[ i ];

	if ( stage.mapType == MAP_TYPE_IMAGE )
	{
        gImageParams_t img;

		// If a texture atlas is being used as a substitute for a texture array,
		// this won't matter.
       
		img.wrap = GL_CLAMP_TO_EDGE;
		img.mipmap = false; //!!( info.localLoadFlags & Q3LOAD_TEXTURE_MIPMAP );

		std::string texFileRoot( map->basePath );
		std::string texRelativePath( &stage.texturePath[ 0 ], strlen( &stage.texturePath[ 0 ] ) );
        texFileRoot.append( texRelativePath );

        // If it's a tga file and we fail, then chances are there is a jpeg duplicate
        // of it that we can fall back on
        if ( !GLoadImageFromFile( texFileRoot.c_str(), img ) )
		{
			std::string ext;
			size_t index = File_GetExt( ext, texFileRoot );
			if ( index != std::string::npos && ext == "tga" )
			{
				texFileRoot.replace( index, 4, ".jpg" );
                if ( !GLoadImageFromFile( texFileRoot, img ) )
				{
					// If we fail second try, turn it into a dummy
					MLOG_WARNING( "Could not load texture file \"%s\"", texFileRoot.c_str() );
                    GSetImageBuffer( img, 64, 64, 4, 255 );
				}
			}
		}

        // We need the highest dimensions out of all images for the texture array
        maxDims.x = glm::max( img.width, maxDims.x );
        maxDims.y = glm::max( img.height, maxDims.y );

        // This index will persist in the texture array it's going into
        stage.textureIndex = images.size();

        images.push_back( img );
	}
}

glm::ivec2 S_LoadShaders( const mapData_t* map, std::vector< gImageParams_t >& textures, shaderMap_t& effectShaders, uint32_t loadFlags )
{
	std::string shaderRootDir( map->basePath );
	shaderRootDir.append( "scripts/" );

    File_IterateDirTree( shaderRootDir, [ & ]( const fileStat_t& f ) -> bool
    {
        std::string ext;
        File_GetExt( ext, f.filepath );

        if ( ext == "shader" )
            ParseShader( effectShaders, loadFlags, f.filepath );

        return true;
    } );

    GenShaderPrograms( effectShaders );

	glm::ivec2 maxDims( 0 );
	for ( auto& entry: effectShaders )
	{
		for ( int i = 0; i < entry.second.stageCount; ++i )
		{
			LoadStageTexture( maxDims, textures, entry.second, i, map );
		}
	}

	return maxDims;
}
