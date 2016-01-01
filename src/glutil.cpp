#include "glutil.h"
#include "q3bsp.h"
#include "shader.h"
#include "aabb.h"
#include "renderer/texture.h"
#include <algorithm>

static std::map< std::string, std::function< void( const Program& program ) > > attribLoadFunctions = 
{
	{
		"position",
		[]( const Program& program ) -> void
		{
			MapVec3( program.attribs.at( "position" ), offsetof( bspVertex_t, position ) );
			GL_CHECK_WITH_NAME( glVertexAttribDivisor( program.attribs.at( "position" ), 0 ), "attribLoadFunctions" ); 
		}
	},
	{
		"normal",
		[]( const Program& program ) -> void
		{
			MapVec3( program.attribs.at( "normal" ), offsetof( bspVertex_t, normal ) );
			GL_CHECK_WITH_NAME( glVertexAttribDivisor( program.attribs.at( "normal" ), 0 ), "attribLoadFunctions" ); 
		}
	},
	{
		"color",
		[]( const Program& program ) -> void
		{
			GL_CHECK_WITH_NAME( glEnableVertexAttribArray( program.attribs.at( "color" ) ), "attribLoadFunctions" ); 
			GL_CHECK_WITH_NAME( glVertexAttribPointer( program.attribs.at( "color" ), 4, GL_UNSIGNED_BYTE, 
				GL_TRUE, sizeof( bspVertex_t ), ( void* ) offsetof( bspVertex_t, color ) ), "attribLoadFunctions" );
			GL_CHECK_WITH_NAME( glVertexAttribDivisor( program.attribs.at( "color" ), 0 ), "attribLoadFunctions" ); 
		}
	},
	{
		"tex0",
		[]( const Program& program ) -> void
		{
			MapVec3( program.attribs.at( "tex0" ), offsetof( bspVertex_t, texCoords[ 0 ] ) );
			GL_CHECK_WITH_NAME( glVertexAttribDivisor( program.attribs.at( "tex0" ), 0 ), "attribLoadFunctions" ); 
		}
	},
	{
		"lightmap",
		[]( const Program& program ) -> void
		{
			MapVec3( program.attribs.at( "lightmap" ), offsetof( bspVertex_t, texCoords[ 1 ] ) );
			GL_CHECK_WITH_NAME( glVertexAttribDivisor( program.attribs.at( "lightmap" ), 0 ), "attribLoadFunctions" ); 
		}
	}
};

GLuint GenSampler( bool mipmap, GLenum wrap )
{
	GLuint sampler;
	GL_CHECK( glGenSamplers( 1, &sampler ) );

	GL_CHECK( glSamplerParameteri( sampler, GL_TEXTURE_MIN_FILTER, mipmap? GL_LINEAR_MIPMAP_LINEAR: GL_LINEAR ) );
	GL_CHECK( glSamplerParameteri( sampler, GL_TEXTURE_MAG_FILTER, GL_LINEAR ) );
	GL_CHECK( glSamplerParameteri( sampler, GL_TEXTURE_WRAP_S, wrap ) );
	GL_CHECK( glSamplerParameteri( sampler, GL_TEXTURE_WRAP_T, wrap ) );
	GL_CHECK( glSamplerParameteri( sampler, GL_TEXTURE_WRAP_R, wrap ) );

	GLfloat maxSamples;
	GL_CHECK( glGetFloatv( GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxSamples ) );
	GL_CHECK( glSamplerParameterf( sampler, GL_TEXTURE_MAX_ANISOTROPY_EXT, maxSamples ) );

	return sampler;
}

void BindTexture( GLenum target, GLuint handle, int32_t offset,  
	int32_t sampler, const std::string& uniform, const Program& program )
{
	GL_CHECK( glActiveTexture( GL_TEXTURE0 + offset ) );
	GL_CHECK( glBindTexture( target, handle ) );
	GL_CHECK( glBindSampler( offset, sampler ) );
	program.LoadInt( uniform, offset );
}

/*
static INLINE void SetPixel( byte* dest, const byte* src, int width, int height, int bpp, int srcX, int srcY, int destX, int destY )
{
	int destOfs = ( width * destY + destX ) * bpp;
	int srcOfs = ( width * srcY + srcX ) * bpp;

	for ( int k = 0; k < bpp; ++k )
		dest[ destOfs + k ] = src[ srcOfs + k ];
}

static INLINE void FlipBytes( byte* out, const byte* src, int width, int height, int bpp )
{
	for ( int y = 0; y < height; ++y )
	{
		for ( int x = 0; x < width; ++x )
		{
			SetPixel( out, src, width, height, bpp, x, height - y - 1, x, y );
		}
	}
}
*/

//-------------------------------------------------------------------------------------------------

texture_t::texture_t( void )
	: srgb( true ), mipmap( false ),
	  handle( 0 ), sampler( 0 ),
	  wrap( GL_REPEAT ), minFilter( GL_LINEAR ), magFilter( GL_LINEAR ), 
	  format( 0 ), internalFormat( 0 ), target( GL_TEXTURE_2D ), maxMip( 0 ),
	  width( 0 ),
	  height( 0 ),
	  depth( 0 ),
	  bpp( 0 )
{
}

texture_t::~texture_t( void )
{
	if ( handle )
	{
		GL_CHECK( glDeleteTextures( 1, &handle ) );
	}

	if ( sampler )
	{
		GL_CHECK( glDeleteSamplers( 1, &sampler ) );
	}
}

void texture_t::Bind( int offset, const std::string& unif, const Program& prog ) const
{
	BindTexture( GL_TEXTURE_2D, handle, offset, sampler, unif, prog );
}

void texture_t::LoadCubeMap( void )
{
	target = GL_TEXTURE_CUBE_MAP;
	
	GenHandle();
	Bind();
	for ( int i = 0; i < 6; ++i )
	{
		GL_CHECK( glTexImage2D( GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, 
			internalFormat, width, height, 0, format, GL_UNSIGNED_BYTE, &pixels[ 0 ] ) );
	}
	Release();

	LoadSettings();
}

void texture_t::Load2D( void )
{
	target = GL_TEXTURE_2D;
	GenHandle();
	Bind();

	if ( mipmap )
	{
		maxMip = Texture_CalcMipLevels2D< texture_t >( *this, width, height, 0 );

		GL_CHECK( glGenerateMipmap( target ) );
		minFilter = GL_LINEAR_MIPMAP_LINEAR;
	}
	else
	{
		GL_CHECK( glTexImage2D( target, 
			0, internalFormat, width, height, 0, format, GL_UNSIGNED_BYTE, &pixels[ 0 ] ) );
	}
	Release();

	LoadSettings();
}

void texture_t::LoadSettings( void )
{
	if ( !sampler )
	{
		sampler = GenSampler( mipmap, wrap );
	}

	GL_CHECK( glBindTexture( target, handle ) );
	GL_CHECK( glTexParameteri( target, GL_TEXTURE_BASE_LEVEL, 0 ) );
	GL_CHECK( glTexParameteri( target, GL_TEXTURE_MAX_LEVEL, maxMip ) );
	GL_CHECK( glBindTexture( target, 0 ) );
}

bool texture_t::LoadFromFile( const char* texPath, uint32_t loadFlags )
{
	std::vector< uint8_t > tmp;
	File_GetPixels( texPath, tmp, bpp, width, height );

    if ( bpp != G_INTERNAL_BPP )
	{
        pixels.resize( width * height * G_INTERNAL_BPP, 255 );
        Pixels_To32Bit( &pixels[ 0 ], &tmp[ 0 ], bpp, width * height );
        bpp = G_INTERNAL_BPP;
	}
	else
	{
		pixels = std::move( tmp );
	}

	if ( !DetermineFormats() )
	{
		MLOG_WARNING( "Unsupported bits per pixel of %i specified; this needs to be fixed. For image file \'%s\'", 
			bpp, texPath );
		return false;
	}
	
	return true;
}

bool texture_t::SetBufferSize( int width0, int height0, int bpp0, byte fill )
{
	width = width0;
	height = height0;
	bpp = bpp0;
	pixels.resize( width * height * bpp, fill );

	return DetermineFormats();
}

bool texture_t::DetermineFormats( void )
{
	switch( bpp )
	{
	case 1:
		format = GL_R;
		internalFormat = GL_R8;
		break;

	case 3:
		format = GL_RGB;
		internalFormat = srgb? GL_SRGB8: GL_RGB8;
		break;

	case 4:
		format = GL_RGBA;
		internalFormat = srgb? GL_SRGB8_ALPHA8: GL_RGBA8;
		break;
	default:
		return false;
		break;
	}

	return true;
}

//-------------------------------------------------------------------------------------------------
// OpenGL 4.x Texture Array
//-------------------------------------------------------------------------------------------------

namespace {

struct mipSetter_t
{
    const GLuint handle;

    const int32_t layerOffset;
    const int32_t numLayers;

    const std::vector< uint8_t >& buffer;

    mipSetter_t(
        const GLuint handle,
        const int32_t layerOffset,
        const int32_t numLayers,
        const std::vector< uint8_t >& buffer );

    void CalcMipLevel2D( int32_t mip, int32_t mipWidth, int32_t mipHeight ) const;
};

mipSetter_t::mipSetter_t(
	const GLuint handle_,
	const int32_t layerOffset_,
	const int32_t numLayers_,
	const std::vector< uint8_t >& buffer_ )

	:	handle( handle_ ),
		layerOffset( layerOffset_ ),
		numLayers( numLayers_ ),
		buffer( buffer_ )
{
}

void mipSetter_t::CalcMipLevel2D( int32_t mip, int32_t mipWidth, int32_t mipHeight ) const
{
	GL_CHECK( glTextureSubImage3D( handle, 
			mip, 0, 0, layerOffset, mipWidth, mipHeight, numLayers, GL_RGBA, GL_UNSIGNED_BYTE, &buffer[ 0 ] ) );
}

}

textureArray_t::textureArray_t( GLsizei width, GLsizei height, GLsizei depth, bool genMipLevels )
	:	megaDims( 
			width, 
			height, 
			depth, 
			genMipLevels? Texture_GetMaxMipLevels2D( width, height ): 1 )
{
	GL_CHECK( glCreateTextures( GL_TEXTURE_2D_ARRAY, 1, &handle ) );

	megaDims.w = glm::min( megaDims.w, GLConfig::MAX_MIP_LEVELS );

	GL_CHECK( glTextureStorage3D( handle, megaDims.w, GL_SRGB8_ALPHA8, megaDims.x, megaDims.y, megaDims.z ) );
	
	std::vector< uint8_t > fill;
	fill.resize( width * height * depth * 4, 255 );
	
	if ( genMipLevels )
	{
		// the vec2 here isn't really necessary, since there is no mip bias for this initial fill
		mipSetter_t ms( handle, 0, depth, fill ); 
        Texture_CalcMipLevels2D< mipSetter_t >( ms, width, height, megaDims.w );
	}
	else
	{
		GL_CHECK( glTextureSubImage3D( handle, 
			0, 0, 0, 0, width, height, depth, GL_RGBA, GL_UNSIGNED_BYTE, &fill[ 0 ] ) );
	}
	
	samplers.resize( megaDims.z, 0 );
	usedSlices.resize( megaDims.z, 0 );
	biases.resize( megaDims.z, glm::vec3( 0.0f ) );
}
	
textureArray_t::~textureArray_t( void )
{
	GL_CHECK( glDeleteTextures( 1, &handle ) );
	GL_CHECK( glDeleteSamplers( samplers.size(), &samplers[ 0 ] ) );
	samplers.clear();
	handle = 0;
}

void textureArray_t::LoadSlice( GLuint sampler, const glm::ivec3& dims, const std::vector< uint8_t >& buffer, bool genMipMaps )
{
	if ( genMipMaps )
	{
		mipSetter_t ms( handle, dims.z, 1, buffer ); 
        Texture_CalcMipLevels2D< mipSetter_t >( ms, dims.x, dims.y, megaDims.w );
	}
	else
	{
		GL_CHECK( glTextureSubImage3D( handle, 0, 0, 0, 
			dims.z, dims.x, dims.y, 1, GL_RGBA, GL_UNSIGNED_BYTE, &buffer[ 0 ] ) );
	}

	samplers[ dims.z ] = sampler;
	usedSlices[ dims.z ] = 1;
	biases[ dims.z ] = glm::vec3( ( float )dims.x / ( float )megaDims.x, ( float )dims.y / ( float )megaDims.y, ( float ) dims.z );
}

void textureArray_t::Bind( GLuint unit, const std::string& samplerName, const Program& program ) const
{
	BindTexture( GL_TEXTURE_2D_ARRAY, handle, unit, 0, samplerName, program );
}
	
void textureArray_t::Release( GLuint unit ) const
{
	GL_CHECK( glActiveTexture( GL_TEXTURE0 + unit ) );
	GL_CHECK( glBindTexture( GL_TEXTURE_2D_ARRAY, 0 ) );
}

//-------------------------------------------------------------------------------------------------

void ImPrep( const glm::mat4& viewTransform, const glm::mat4& clipTransform )
{
	GL_CHECK( glUseProgram( 0 ) );
	GL_CHECK( glMatrixMode( GL_PROJECTION ) );
	GL_CHECK( glLoadIdentity() );
	GL_CHECK( glLoadMatrixf( glm::value_ptr( clipTransform ) ) );
	GL_CHECK( glMatrixMode( GL_MODELVIEW ) );
	GL_CHECK( glLoadIdentity() );
	GL_CHECK( glLoadMatrixf( glm::value_ptr( viewTransform ) ) );
}

void ImDrawAxes( const float size ) 
{
	std::array< glm::vec3, 6 > axes = 
	{
		glm::vec3( size, 0.0f, 0.0f ), glm::vec3( 1.0f, 0.0f, 0.0f ),
		glm::vec3( 0.0f, size, 0.0f ), glm::vec3( 0.0f, 1.0f, 0.0f ),
		glm::vec3( 0.0f, 0.0f, -size ), glm::vec3( 0.0f, 0.0f, 1.0f )
	};
	
	glBegin( GL_LINES );
	for ( int i = 0; i < 6; i += 2 )
	{
		glColor3fv( glm::value_ptr( axes[ i + 1 ] ) );
		glVertex3f( 0.0f, 0.0f, 0.0f );
		glVertex3fv( glm::value_ptr( axes[ i ] ) ); 
	}
	glEnd();
}

void ImDrawBounds( const AABB& bounds, const glm::vec4& color )
{
	const glm::vec3 center( bounds.Center() );

	const std::array< const glm::vec3, 8 > points = 
	{
		bounds.maxPoint - center, 
		glm::vec3( bounds.maxPoint.x, bounds.maxPoint.y, bounds.minPoint.z ) - center,
		glm::vec3( bounds.maxPoint.x, bounds.minPoint.y, bounds.minPoint.z ) - center, 
		glm::vec3( bounds.maxPoint.x, bounds.minPoint.y, bounds.maxPoint.z ) - center,
		
		bounds.minPoint - center,
		glm::vec3( bounds.minPoint.x, bounds.maxPoint.y, bounds.minPoint.z ) - center,
		glm::vec3( bounds.minPoint.x, bounds.minPoint.y, bounds.minPoint.z ) - center, 
		glm::vec3( bounds.minPoint.x, bounds.minPoint.y, bounds.maxPoint.z ) - center,
	};
	
	GL_CHECK( glMatrixMode( GL_MODELVIEW ) );
	GL_CHECK( glPushMatrix() );
	GL_CHECK( glTranslatef( center.x, center.y, center.z ) );
	
	glBegin( GL_LINE_STRIP );
	glColor4fv( glm::value_ptr( color ) );

	for ( int i = 0; i < 8; ++i )
	{
		glVertex3fv( glm::value_ptr( points[ i ] ) );
	}

	glEnd();
	GL_CHECK( glPopMatrix() );
}

void ImDrawPoint( const glm::vec3& point, const glm::vec4& color, float size )
{
	GL_CHECK( glMatrixMode( GL_MODELVIEW ) );
	GL_CHECK( glPushMatrix() );
	GL_CHECK( glTranslatef( point.x, point.y, point.z ) );
	GL_CHECK( glPushAttrib( GL_POINT_BIT ) );
	GL_CHECK( glPointSize( size ) );

	glBegin( GL_POINTS );
	glColor4fv( glm::value_ptr( color ) );
	glVertex3f( 0.0f, 0.0f, 0.0f );
	glEnd();

	GL_CHECK( glPopAttrib() );
	GL_CHECK( glPopMatrix() );
}

void SetPolygonOffsetState( bool enable, uint32_t polyFlags )
{
	if ( enable )
	{
		if ( polyFlags & GLUTIL_POLYGON_OFFSET_FILL ) GL_CHECK( glEnable( GL_POLYGON_OFFSET_FILL ) );
		if ( polyFlags & GLUTIL_POLYGON_OFFSET_LINE ) GL_CHECK( glEnable( GL_POLYGON_OFFSET_LINE ) );
		if ( polyFlags & GLUTIL_POLYGON_OFFSET_POINT ) GL_CHECK( glEnable( GL_POLYGON_OFFSET_POINT ) );
	}
	else
	{
		if ( polyFlags & GLUTIL_POLYGON_OFFSET_FILL ) GL_CHECK( glDisable( GL_POLYGON_OFFSET_FILL ) );
		if ( polyFlags & GLUTIL_POLYGON_OFFSET_LINE ) GL_CHECK( glDisable( GL_POLYGON_OFFSET_LINE ) );
		if ( polyFlags & GLUTIL_POLYGON_OFFSET_POINT ) GL_CHECK( glDisable( GL_POLYGON_OFFSET_POINT ) );
	}
}

//-------------------------------------------------------------------------------------------------

Program::Program( const std::string& vertexShader, const std::string& fragmentShader )
	: program( 0 )
{
	GLuint shaders[] = 
	{
		CompileShaderSource( vertexShader.c_str(), vertexShader.size(), GL_VERTEX_SHADER ),
		CompileShaderSource( fragmentShader.c_str(), fragmentShader.size(), GL_FRAGMENT_SHADER )
	};

	program = LinkProgram( shaders, 2 );
}

Program::Program( const std::string& vertexShader, const std::string& fragmentShader, 
	const std::vector< std::string >& uniforms, const std::vector< std::string >& attribs, bool bindTransformsUbo )
	: Program( vertexShader, fragmentShader )
{
	GenData( uniforms, attribs, bindTransformsUbo );
}

Program::Program( const std::vector< char >& vertexShader, const std::vector< char >& fragmentShader, 
		const std::vector< std::string >& uniforms, const std::vector< std::string >& attribs, bool bindTransformsUbo )
		: Program( std::string( &vertexShader[ 0 ], vertexShader.size() ), 
				std::string( &fragmentShader[ 0 ], fragmentShader.size() ) )
{
	GenData( uniforms, attribs, bindTransformsUbo );
}

Program::Program( const Program& copy )
	: program( copy.program ),
	  uniforms( copy.uniforms ),
	  attribs( copy.attribs )
{
}

Program::~Program( void )
{
	Release();
	GL_CHECK( glDeleteProgram( program ) );
}

void Program::GenData( const std::vector< std::string >& uniforms, 
	const std::vector< std::string >& attribs, bool bindTransformsUbo )
{
	uint32_t max = glm::max( attribs.size(), uniforms.size() );
	for ( uint32_t i = 0; i < max; ++i )
	{
		if ( i < attribs.size() )
		{
			AddAttrib( attribs[ i ] );
		}

		if ( i < uniforms.size() )
		{
			AddUnif( uniforms[ i ] );
		}
	}

	if ( bindTransformsUbo )
	{
		MapProgramToUBO( program, "Transforms" );
	}
}

void Program::LoadAttribLayout( void ) const
{
	for ( int i = 0; i < 5; ++i )
	{
		GL_CHECK( glDisableVertexAttribArray( i ) );
	}

    for ( const auto& attrib: attribs )
	{
		if ( attrib.second != -1 )
		{

            auto it = std::find( disableAttribs.begin(), disableAttribs.end(), attrib.first );

            if ( it != disableAttribs.end() )
            {
                GL_CHECK( glDisableVertexAttribArray( attrib.second ) );
                continue;
            }
			
			attribLoadFunctions[ attrib.first ]( *this ); 
		}
	}
}

std::vector< std::string > Program::ArrayLocationNames( const std::string& name, int32_t length )
{
	std::vector< std::string > names;
	names.resize( length );

	for ( int32_t i = 0; i < length; ++i )
	{
		names[ i ] = name + "[" + std::to_string( i ) + "]";
	}
	return names;
}

//-------------------------------------------------------------------------------------------------

loadBlend_t::loadBlend_t( GLenum srcFactor, GLenum dstFactor )
{
	GL_CHECK( glGetIntegerv( GL_BLEND_SRC_RGB, ( GLint* ) &prevSrcFactor ) );
	GL_CHECK( glGetIntegerv( GL_BLEND_DST_RGB, ( GLint* ) &prevDstFactor ) );

	GL_CHECK( glBlendFunc( srcFactor, dstFactor ) );
}

loadBlend_t::~loadBlend_t( void )
{
	GL_CHECK( glBlendFunc( prevSrcFactor, prevDstFactor ) );
}

//-------------------------------------------------------------------------------------------------


