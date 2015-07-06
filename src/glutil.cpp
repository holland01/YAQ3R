#include "glutil.h"
#include "q3bsp.h"
#include "shader.h"
#include "aabb.h"

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
			MapAttribTexCoord( program.attribs.at( "tex0" ), offsetof( bspVertex_t, texCoords[ 0 ] ) );
			GL_CHECK_WITH_NAME( glVertexAttribDivisor( program.attribs.at( "tex0" ), 0 ), "attribLoadFunctions" ); 
		}
	},
	{
		"lightmap",
		[]( const Program& program ) -> void
		{
			MapAttribTexCoord( program.attribs.at( "lightmap" ), offsetof( bspVertex_t, texCoords[ 1 ] ) );
			GL_CHECK_WITH_NAME( glVertexAttribDivisor( program.attribs.at( "lightmap" ), 0 ), "attribLoadFunctions" ); 
		}
	}
};

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

// Assumes square dimensions; solution based off of an image rotation algorithm from "Cracking the Coding Interview"
static INLINE void RotateSquareImage90CCW( std::vector< byte >& image, int dims, int bpp )
{
	const int layerCount = dims / 2;
	for ( int y = dims - 1; y >= layerCount; --y )
	{
		int upMost = y;
		int downMost = dims - y - 1;
		for ( int x = downMost; x <= upMost; ++x )
		{
			// Pixel Base: ( width * row + column ) * bytes per pixel

			int up = ( dims * upMost + dims - x - 1 ) * bpp; 
			int down = ( dims * downMost + x ) * bpp;
			int left = ( dims * ( dims - x - 1 ) + downMost ) * bpp;
			int right = ( dims * x + upMost ) * bpp;

			byte tmp[ 4 ] = { 0, 0, 0, 0 };
			memcpy( tmp, &image[ left ], sizeof( byte ) * bpp );
			
			memcpy( &image[ left ], &image[ up ], sizeof( byte ) * bpp );
			memcpy( &image[ up ], &image[ right ], sizeof( byte ) * bpp );
			memcpy( &image[ right ], &image[ down ], sizeof( byte ) * bpp );
			memcpy( &image[ down ], &tmp, sizeof( byte ) * bpp );
		}
	}
}

texture_t::texture_t( void )
	: srgb( true ), mipmap( false ),
	  handle( 0 ), sampler( 0 ),
	  wrap( GL_CLAMP_TO_EDGE ), minFilter( GL_LINEAR ), magFilter( GL_LINEAR ), 
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
	GL_CHECK( glActiveTexture( GL_TEXTURE0 + offset ) );
	GL_CHECK( glBindTexture( target, handle ) );
	GL_CHECK( glBindSampler( offset, sampler ) );
	prog.LoadInt( unif, offset );
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
		int maxLevels = glm::min( ( int ) glm::log2( ( float ) width ), ( int ) glm::log2( ( float ) height ) ); 

		int w = width;
		int h = height;
		int mip;

		for ( mip = 0; h != 1 || w != 1; ++mip )
		{
			GL_CHECK( glTexImage2D( target, 
				mip, internalFormat, w, h, 0, format, GL_UNSIGNED_BYTE, &pixels[ 0 ] ) );

			if ( h > 1 )
			{
				h /= 2;
			}

			if ( w > 1 )
			{
				w /= 2;
			}
		}

		GL_CHECK( glGenerateMipmap( target ) );
		minFilter = GL_LINEAR_MIPMAP_LINEAR;
		maxMip = mip;
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
		GL_CHECK( glGenSamplers( 1, &sampler ) );
	}

	GL_CHECK( glSamplerParameteri( sampler, GL_TEXTURE_MIN_FILTER, minFilter ) );
	GL_CHECK( glSamplerParameteri( sampler, GL_TEXTURE_MAG_FILTER, magFilter ) );
	GL_CHECK( glSamplerParameteri( sampler, GL_TEXTURE_WRAP_S, wrap ) );
	GL_CHECK( glSamplerParameteri( sampler, GL_TEXTURE_WRAP_T, wrap ) );
	GL_CHECK( glSamplerParameteri( sampler, GL_TEXTURE_WRAP_R, wrap ) );

	GLfloat maxSamples;
	GL_CHECK( glGetFloatv( GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxSamples ) );
	GL_CHECK( glSamplerParameterf( sampler, GL_TEXTURE_MAX_ANISOTROPY_EXT, maxSamples ) );

	GL_CHECK( glBindTexture( target, handle ) );
	GL_CHECK( glTexParameteri( target, GL_TEXTURE_BASE_LEVEL, 0 ) );
	GL_CHECK( glTexParameteri( target, GL_TEXTURE_MAX_LEVEL, maxMip ) );
	GL_CHECK( glBindTexture( target, 0 ) );
}

bool texture_t::LoadFromFile( const char* texPath, uint32_t loadFlags )
{
	std::vector< uint8_t > tmp;
	File_GetPixels( texPath, tmp, bpp, width, height );

	if ( bpp == 3 )
	{
		pixels.resize( width * height * 4, 255 ); 
		Pixels_24BitTo32Bit( &pixels[ 0 ], &tmp[ 0 ], width * height );
		bpp = 4;
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
		
	if ( loadFlags & Q3LOAD_TEXTURE_ROTATE90CCW )
	{
		RotateSquareImage90CCW( pixels, width, bpp );
	}

	Load2D();

	//LoadSettings();
	//Load2D();
	
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

void LoadVertexLayout( uint32_t attribFlags, const Program& prog )
{
	for ( GLuint i = 0; i < GLUTIL_NUM_ATTRIBS_MAX; ++i )
	{
		GL_CHECK( glDisableVertexAttribArray( i ) );
	}

	if ( attribFlags & GLUTIL_LAYOUT_POSITION ) 
	{
		MapVec3( prog.attribs.at( "position" ), offsetof( bspVertex_t, position ) );
		GL_CHECK( glVertexAttribDivisor( prog.attribs.at( "position" ), 0 ) ); 
	}

	if ( attribFlags & GLUTIL_LAYOUT_NORMAL )
	{
		MapVec3( prog.attribs.at( "normal" ), offsetof( bspVertex_t, normal ) );
		GL_CHECK( glVertexAttribDivisor( prog.attribs.at( "normal" ), 0 ) ); 
	}

	if ( attribFlags & GLUTIL_LAYOUT_COLOR )
	{
		GL_CHECK( glEnableVertexAttribArray( prog.attribs.at( "color" ) ) ); 
		GL_CHECK( glVertexAttribPointer( prog.attribs.at( "color" ), 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof( bspVertex_t ), ( void* ) offsetof( bspVertex_t, color ) ) );
		GL_CHECK( glVertexAttribDivisor( prog.attribs.at( "color" ), 0 ) ); 
	}

	if ( attribFlags & GLUTIL_LAYOUT_TEX0 )
	{
		MapAttribTexCoord( prog.attribs.at( "tex0" ), offsetof( bspVertex_t, texCoords[ 0 ] ) );
		GL_CHECK( glVertexAttribDivisor( prog.attribs.at( "tex0" ), 0 ) ); 
	}

	if ( attribFlags & GLUTIL_LAYOUT_LIGHTMAP )
	{
		MapAttribTexCoord( prog.attribs.at( "lightmap" ), offsetof( bspVertex_t, texCoords[ 1 ] ) );
		GL_CHECK( glVertexAttribDivisor( prog.attribs.at( "lightmap" ), 0 ) ); 
	}
}

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

// -------------------------------------------------------------------------------------------------
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
			if ( !disableAttribs.empty() )
			{
				auto it = std::find( disableAttribs.begin(), disableAttribs.end(), attrib.first );

				if ( it != disableAttribs.end() )
				{
					GL_CHECK( glDisableVertexAttribArray( attrib.second ) );
					continue;
				}
			}
			
			attribLoadFunctions[ attrib.first ]( *this ); 
		}
	}
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
TextureBuffer::TextureBuffer( GLsizei width, GLsizei height, GLsizei depth, GLsizei mipLevels )
	: megaDims( width, height, depth )
{
	GL_CHECK( glCreateTextures( GL_TEXTURE_2D_ARRAY, 1, &handle ) );
	GL_CHECK( glTextureStorage3D( handle, mipLevels, GL_SRGB8_ALPHA8, width, height, depth ) );
	
	data.reserve( megaDims.z );
	pixels.reserve( width * height * depth * 4 );
}
	
TextureBuffer::~TextureBuffer( void )
{
	GL_CHECK( glDeleteTextures( 1, &handle ) );
}

void TextureBuffer::AddBuffer( GLsizei level, GLuint sampler, glm::ivec3& dims, const std::vector< uint8_t >& buffer )
{
	if ( data.size() == megaDims.z )
	{
		__nop();
	}

	pixels.insert( pixels.end(), buffer.begin(), buffer.end() );
	
	dims.z = data.size();
	GL_CHECK( glTextureSubImage3D( handle, 
		level, 0, 0, dims.z, dims.x, dims.y, 1, GL_RGBA, GL_UNSIGNED_BYTE, &buffer[ 0 ] ) ); 

	textureData_t entry = 
	{
		sampler,
		dims
	};

	data.push_back( std::move( entry ) );
}
