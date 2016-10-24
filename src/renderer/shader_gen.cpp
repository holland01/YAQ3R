#include "shader_gen.h"
#include "effect_shader.h"

//--------------------------------------------------
// DEBUG
//--------------------------------------------------

struct meta_t
{
	bool logProgramGen = true;
	FILE*	programLog = nullptr;
	uint32_t currLineCount = 0;

	std::string currShaderFile;

	meta_t( void )
	{
		if ( logProgramGen )
			programLog = fopen( "log/shader_gen.txt", "wb" );
	}

	~meta_t( void )
	{
		if ( programLog )
			fclose( programLog );
	}

	void LogShader( const std::string& title,
					const std::string& source,
					uint8_t stage = 0 )
	{
		if ( logProgramGen )
		{
			fprintf( programLog, "[ %i ] [ \n\n %s \n\n%s \n\n ]",
				stage,
				title.c_str(),
				source.c_str() );
		}
	}

	void LogEndShaderGen( void )
	{
		if ( logProgramGen )
		{
			fprintf( programLog,
					 "---------------------------\n\n"
					 "PROGRAM COUNT: %llu"
					 "\n\n-----------------------------",
					 ( unsigned long long ) GNumPrograms() );
		}
	}
};

static std::unique_ptr< meta_t > gMeta( new meta_t() );

//--------------------------------------------------
// Shader Gen Utilities
//--------------------------------------------------

static INLINE void AddDiscardIf( std::vector< std::string >& fragmentSrc, const std::string& discardPredicate )
{
	fragmentSrc.push_back( "\tif ( " + discardPredicate + " )" );
	fragmentSrc.push_back( "\t{" );
	fragmentSrc.push_back( "\t\tdiscard;" );
	fragmentSrc.push_back( "\t}" );
}

static INLINE std::string SampleTexture2D( const std::string& samplerName, const std::string& coords )
{
#ifdef G_USE_GL_CORE
	std::string fname( "texture" );
#else
	std::string fname( "texture2D" );
#endif
	return fname + "( " + samplerName + ", " + coords + " )";
}

static INLINE std::string SampleFromTexture( const std::string& destCoords,
									 const std::string& destSample,
									 const std::string& srcCoords,
									 const std::string& srcSample,
									 const std::string& scaleParams,
									 const std::string& transform )
{

	std::stringstream ss;

	ss << "\t" << destCoords << " = " << "mod( " << srcCoords << ", vec2( 0.99 ) ) * " << scaleParams
	   << " * " << transform << ".zw + " << transform << ".xy;\n";

	ss << "\t" << destSample << " = " << SampleTexture2D( srcSample, destCoords ) << ";";

	return ss.str();
}


static INLINE std::string WriteFragment( const std::string& value )
{
#ifdef G_USE_GL_CORE
	return "fragment = " + value + ";";
#else
	return "gl_FragColor = " + value + ";";
#endif
}

static INLINE std::string DeclAttributeVar( const std::string& name, const std::string& type, const int32_t location = -1 )
{
#ifdef G_USE_GL_CORE
	std::string decl("in " + type + " " + name + ";");

#	ifdef __linux__
	UNUSED( location );
#else
	if ( location != -1 )
	{
		decl = "layout( location = " + std::to_string( location ) + " ) " + decl;
	}
#	endif
	return decl;
#else
	UNUSED( location );
	return "attribute " + type + " " + name + ";";
#endif
}

static INLINE std::string DeclTransferVar( const std::string& name,
										   const std::string& type,
										   const std::string& transferMode="" )
{
#ifdef G_USE_GL_CORE
	return transferMode + " " + type + " " + name + ";";
#else
	UNUSED( transferMode );
	return "varying " + type + " " + name + ";";
#endif
}

static INLINE std::string DeclCoreTransforms( void )
{
	return "uniform mat4 modelToView;\n"
		   "uniform mat4 viewToClip;";
}

static INLINE std::string DeclGammaConstant( void )
{
	return "const float gamma = 1.0 / 3.3;";
}

static INLINE std::string DeclPrecision( void )
{
#ifdef G_USE_GL_CORE
	return "\n";
#else
	return "precision highp float;";
#endif
}

static INLINE std::string GammaCorrect( const std::string& colorVec )
{
	std::stringstream ss;

	ss << "\t" << colorVec << ".r = pow( " << colorVec << ".r, gamma );\n"
	   << "\t" << colorVec << ".g = pow( " << colorVec << ".g, gamma );\n"
	   << "\t" << colorVec << ".b = pow( " << colorVec << ".b, gamma );";

	return ss.str();
}

static INLINE bool UsesColor( const shaderStage_t& stage )
{
	return stage.rgbGen == RGBGEN_VERTEX;
}

static INLINE void WriteTexture(
		std::vector< std::string >& fragmentSrc,
		const shaderStage_t& stage,
		const char* discardPredicate )
{
	std::string sampleTextureExpr;

	if ( stage.mapCmd == MAP_CMD_CLAMPMAP )
	{
		fragmentSrc.push_back( "\tst = clamp( applyTransform( st )," \
			 "imageTransform.xy, applyTransform( vec2( 0.99 ) ) );" );
	}
	else
	{
		fragmentSrc.push_back( "\tst = applyTransform( mod( st, vec2( 0.99 ) ) );" );
	}

	sampleTextureExpr = SampleTexture2D( "sampler0", "st" );

	std::stringstream colorAssign;

	colorAssign << "\tvec4 color = ";

	// Some shader entries will incorporate specific alpha values
	if ( stage.alphaGen != 0.0f )
	{
		fragmentSrc.push_back( "\tconst float alphaGen = "
			+ std::to_string( stage.alphaGen ) + std::to_string( ';' ) );

		colorAssign << sampleTextureExpr << ".rgb, alphaGen )";

		if ( UsesColor( stage ) )
		{
			 colorAssign << " * vec4( frag_Color.rgb, alphaGen )";
		}
	}
	else
	{
		colorAssign << sampleTextureExpr;

		if ( UsesColor( stage ) )
		{
			colorAssign <<  " * frag_Color";
		}
	}

	colorAssign << ";";

	fragmentSrc.push_back( colorAssign.str() );

	// Is used occasionally, for example in situations like a bad alpha value.
	if ( discardPredicate )
		AddDiscardIf( fragmentSrc, std::string( discardPredicate ) );

	// Gamma correction
	fragmentSrc.insert( fragmentSrc.end(), GammaCorrect( "color" ) );

	fragmentSrc.push_back( "\t" + WriteFragment( "color" ) );
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
		"\tvec3 R = 2.0 * " + normalID + " * dot( dirToEye, " + normalID + " ) - dirToEye;",
		"\tvec2 displace = R.yz * 0.5;",
		"\tvec2 st = vec2( 0.5 ) + vec2( displace.x, -displace.y );"
	} );
}

static INLINE std::string JoinLines( std::vector< std::string >& lines )
{
	std::stringstream shaderSrc;

	for ( std::string& line: lines )
		shaderSrc << line << '\n';

	// Append the end bracket for the main function
	shaderSrc << '}';

	return shaderSrc.str();
}

static std::string GenVertexShader( shaderStage_t& stage,
									const std::string& texCoordName,
									std::vector< std::string >& attribs )
{

	size_t vertTransferOffset = 3;
	size_t vertAttrOffset = 2;

	uint32_t attribLocCounter = 0;

	// Vertex shader...
	// if we're not using core the last args for both the attribute and transfer
	// decl funcs won't be written; they'll likely just get optimized away
	std::vector< std::string > vertexSrc =
	{
		DeclAttributeVar( "position", "vec3", attribLocCounter++ ),
		DeclAttributeVar( texCoordName, "vec2", attribLocCounter++ ),
		DeclTransferVar( "frag_Tex", "vec2", "out" ),
		DeclCoreTransforms(),
		"void main(void) {",
	};

	if ( stage.tcgen == TCGEN_ENVIRONMENT )
	{
		vertexSrc.insert( vertexSrc.begin() + vertAttrOffset++,
			DeclAttributeVar( "normal", "vec3", attribLocCounter++ ) );
		attribs.push_back( "normal" );
	}

	vertexSrc.push_back(
		"\tgl_Position = viewToClip * modelToView * vec4( position, 1.0 );" );

	if ( stage.tcgen == TCGEN_ENVIRONMENT )
	{
		AddCalcEnvMap( vertexSrc, "position", "normal",
			"vec3( -modelToView[ 3 ] )" );
		vertexSrc.push_back( "\tfrag_Tex = st;" );
	}
	else
	{
		vertexSrc.push_back( "\tfrag_Tex = " + texCoordName + ";" );
	}

	if ( UsesColor( stage ) )
	{
		vertexSrc.insert( vertexSrc.begin() + vertAttrOffset++,
			DeclAttributeVar( "color", "vec4", attribLocCounter++ ) );
		vertexSrc.insert( vertexSrc.begin() + vertTransferOffset++,
			DeclTransferVar( "frag_Color", "vec4", "out" ) );
		vertexSrc.push_back( "\tfrag_Color = color;" );
		attribs.push_back( "color" );
	}

	return JoinLines( vertexSrc );
}

static std::string GenFragmentShader( shaderStage_t& stage,
							   std::vector< std::string >& uniforms )
{
	size_t fragTransferOffset = 2;
	size_t fragUnifOffset = 3;

	// Fragment shader....
	// Unspecified alphaGen implies a default 1.0 alpha channel
	std::vector< std::string > fragmentSrc =
	{
		DeclPrecision(),
		"const float gamma = 1.0 / 2.2;",
		DeclTransferVar( "frag_Tex", "vec2", "in" ),
#ifdef G_USE_GL_CORE
		DeclTransferVar( "fragment", "vec4", "out" ),
#endif
		"void main(void) {"
	};

	// Add our base image data
	std::initializer_list<std::string> data  =
	{
		"uniform sampler2D sampler0;",
		"uniform vec4 imageTransform;",
		"uniform vec2 imageScaleRatio;",
		"vec2 applyTransform(in vec2 coords) {",
		"\treturn coords * imageTransform.zw * imageScaleRatio + imageTransform.xy;",
		"}"
	};

	fragmentSrc.insert( fragmentSrc.begin() + fragUnifOffset, data );

	fragmentSrc.push_back( "\tvec2 st = frag_Tex;" );

	if ( UsesColor( stage ) )
	{
		fragmentSrc.insert( fragmentSrc.begin() + fragTransferOffset++,
			DeclTransferVar( "frag_Color", "vec4", "in" ) );
	}

	for ( const effect_t& op: stage.effects )
	{
		// Modify the texture coordinate as necessary before we write to the texture
		if ( op.name == "tcModTurb" )
		{
			fragmentSrc.insert( fragmentSrc.begin() + fragUnifOffset,
				"uniform float tcModTurb;" );
			fragmentSrc.push_back( "\tst *= tcModTurb;" );
			uniforms.push_back( "tcModTurb" );
		}
		else if ( op.name == "tcModScroll" )
		{
			fragmentSrc.insert( fragmentSrc.begin() + fragUnifOffset,
				"uniform vec4 tcModScroll;" );
			fragmentSrc.push_back( "\tst += tcModScroll.xy * tcModScroll.zw;" );
			uniforms.push_back( "tcModScroll" );
		}
		else if ( op.name == "tcModRotate" )
		{
			fragmentSrc.insert( fragmentSrc.begin() + fragUnifOffset,
				"uniform mat2 texRotate;" );
			fragmentSrc.insert( fragmentSrc.begin() + fragUnifOffset,
				"uniform vec2 texCenter;" );
			fragmentSrc.push_back( "\tst += texRotate * ( frag_Tex - texCenter );" );

			uniforms.push_back( "texRotate" );
			uniforms.push_back( "texCenter" );
		}
		else if ( op.name == "tcModScale" )
		{
			fragmentSrc.insert( fragmentSrc.begin() + fragUnifOffset,
				"uniform mat2 tcModScale;" );
			fragmentSrc.push_back( "\tst = tcModScale * st;" );
			uniforms.push_back( "tcModScale" );
		}
	}

	switch ( stage.alphaFunc )
	{
	case ALPHA_FUNC_UNDEFINED:
		WriteTexture( fragmentSrc, stage, nullptr );
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

	return JoinLines( fragmentSrc );
}

//--------------------------------------------------
// Shader Gen API
//--------------------------------------------------

std::string GGetGLSLHeader( void )
{
#ifdef G_USE_GL_CORE
#	ifdef __linux__
		return "#version 130\n";
#	else
		return "#version 330";
#	endif // __linux__
#else
	return "#version 100";
#endif
}

//-------
// Static shader generation functions
//------

//! JoinLines function call will append
//! the ending bracket for the main function,
//! so we don't have to worry about it here.
std::string GMakeMainVertexShader( void )
{
	std::vector< std::string > sourceLines =
	{
		DeclAttributeVar( "position", "vec3", 0 ),
		DeclAttributeVar( "tex0", "vec2", 1 ),
		DeclAttributeVar( "lightmap", "vec2", 2 ),
		DeclAttributeVar( "color", "vec4", 3 ),
		DeclTransferVar( "frag_Color", "vec4", "out" ),
		DeclTransferVar( "frag_Tex", "vec2", "out" ),
		DeclTransferVar( "frag_Lightmap", "vec2", "out" ),
		DeclCoreTransforms(),
		"void main(void) {",
		"\tgl_Position = viewToClip * modelToView * vec4( position, 1.0 );",
		"\tfrag_Color = color;",
		"\tfrag_Lightmap = lightmap;",
		"\tfrag_Tex = tex0;"
	};

	std::string source( JoinLines( sourceLines ) );

	return source;
}

std::string GMakeMainFragmentShader( void )
{
	std::vector< std::string > sourceLines =
	{
		DeclPrecision(),
		DeclGammaConstant(),
		DeclTransferVar( "frag_Color", "vec4", "in" ),
		DeclTransferVar( "frag_Tex", "vec2", "in" ),
		DeclTransferVar( "frag_Lightmap", "vec2", "in" ),
		"uniform sampler2D mainImageSampler;",
		"uniform vec2 mainImageImageScaleRatio;",
		"uniform vec4 mainImageImageTransform;",
		"uniform sampler2D lightmapSampler;",
		"uniform vec2 lightmapImageScaleRatio;",
		"uniform vec4 lightmapImageTransform;",
#ifdef G_USE_GL_CORE
		DeclTransferVar( "fragment", "vec4", "out" ),
#endif
		"void main(void) {",
		"\tvec2 texCoords;",
		"\tvec4 image, lightmap, color;",
		SampleFromTexture( "texCoords", "image", "frag_Tex",
			"mainImageSampler", "mainImageImageScaleRatio", "mainImageImageTransform" ),
		SampleFromTexture( "texCoords", "lightmap", "frag_Lightmap",
			"lightmapSampler", "lightmapImageScaleRatio", "lightmapImageTransform" ),
		"\tcolor = frag_Color * image * lightmap;",
		GammaCorrect( "color" ),
		WriteFragment( "color" )
	};

	std::string source( JoinLines( sourceLines ) );

	return source;
}

void GMakeProgramsFromEffectShader( shaderInfo_t& shader )
{
	int j;
	for ( j = 0; j < shader.stageCount; ++j )
	{
		shaderStage_t& stage = shader.stageBuffer[ j ];

		const std::string texCoordName( ( stage.mapType == MAP_TYPE_LIGHT_MAP )?
				"lightmap": "tex0" );

		std::vector< std::string > attribs = { "position", texCoordName };
		std::vector< std::string > uniforms = {
			"sampler0",
			"imageTransform",
			"imageScaleRatio",
			"modelToView",
			"viewToClip",
		};

		const std::string& vertexString = GenVertexShader( stage, texCoordName,
				attribs );
		const std::string& fragmentString = GenFragmentShader( stage, uniforms );

		Program* p = new Program( vertexString, fragmentString, uniforms, attribs );

		// On the directive: this is good for testing and
		// doing performance comparisons
#ifndef G_DUPLICATE_PROGRAMS
		stage.program = GFindProgramByData( p->attribs, p->uniforms, &stage );
		if ( G_HNULL( stage.program ) )
#endif

		{
			gMeta->LogShader( "Vertex", vertexString, j );
			gMeta->LogShader( "Fragment", fragmentString, j );
			p->stage = &stage;
			stage.program = GStoreProgram( p );
		}

#ifndef G_DUPLICATE_PROGRAMS
		else
		{
			delete p;
		}
#endif
	}
}
