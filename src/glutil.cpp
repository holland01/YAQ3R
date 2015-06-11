#include "glutil.h"
#include "q3bsp.h"
#include "shader.h"
#include "extern/stb_image.c"

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
	: srgb( true ),
	  handle( 0 ), sampler( 0 ),
	  wrap( 0 ), format( 0 ), internalFormat( 0 ),
	  width( 0 ),
	  height( 0 ),
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

void Tex_SetBufferSize( texture_t& tex, int width, int height, int bpp, byte fill )
{
	tex.width = width;
	tex.height = height;
	tex.bpp = bpp;
	tex.pixels.resize( tex.width * tex.height * tex.bpp, fill );

	switch( tex.bpp )
	{
	case 1:
		tex.format = GL_R;
		tex.internalFormat = GL_R8;
		break;

	case 3:
		tex.format = GL_RGB;
		tex.internalFormat = tex.srgb? GL_SRGB8 : GL_RGB8;
		break;

	case 4:
		tex.format = GL_RGBA;
		tex.format = tex.srgb? GL_SRGB8_ALPHA8 : GL_RGBA8;
		break;
	}
}

void Tex_MakeTexture2D( texture_t& tex )
{
	if ( !tex.handle )
	{
		GL_CHECK( glGenTextures( 1, &tex.handle ) );
	}

	GL_CHECK( glBindTexture( GL_TEXTURE_2D, tex.handle ) );
	GL_CHECK( glTexImage2D( 
		GL_TEXTURE_2D, 
		0, 
		tex.internalFormat, 
		tex.width, 
		tex.height, 
		0, 
		tex.format, 
		GL_UNSIGNED_BYTE, 
		&tex.pixels[ 0 ] ) );	
	GL_CHECK( glBindTexture( GL_TEXTURE_2D, 0 ) );
}
  
bool LoadTextureFromFile( const char* texPath, uint32_t loadFlags, texture_t& texture )
{
	// Load image
	// Need to also flip the image, since stbi loads pointer to upper left rather than lower left (what OpenGL expects)
	{
		byte* imagePixels = stbi_load( texPath, &texture.width, &texture.height, &texture.bpp, STBI_default );

		if ( !imagePixels )
		{
			MLOG_WARNING( "No file found for \'%s\'", texPath );
			return false;
		}
		
		texture.pixels.resize( texture.width * texture.height * texture.bpp, 0 );
		FlipBytes( &texture.pixels[ 0 ], imagePixels, texture.width, texture.height, texture.bpp );	
		
		if ( loadFlags & Q3LOAD_TEXTURE_ROTATE90CCW )
		{
			RotateSquareImage90CCW( texture.pixels, texture.width, texture.bpp );
		}

		stbi_image_free( imagePixels );
	}

	// NOTE: OpenGL expects image to have linear colorspace on input, hence why SRGB is only taken into 
	// account in terms of the internal format
	GLenum rgb, rgba;
	if ( loadFlags & Q3LOAD_TEXTURE_SRGB )
	{
		rgb = GL_SRGB8;
		rgba = GL_SRGB8_ALPHA8;
	}
	else
	{
		rgb = GL_RGB8;
		rgba = GL_RGBA8;
	}

	switch ( texture.bpp )
	{
	case 1:
		texture.format = GL_R;
		texture.internalFormat = GL_R8; 
		break;
	case 3:
		texture.format  = GL_RGB;
		texture.internalFormat = rgb;
		break;
	case 4:
		texture.format  = GL_RGBA;
		texture.internalFormat = rgba;
		break;
	default:
		MLOG_ERROR( "Unsupported bits per pixel of %i specified; this needs to be fixed. For image file \'%s\'", 
			texture.bpp, texPath );
		break;
	}

	if ( !texture.handle )
	{
		GL_CHECK( glGenTextures( 1, &texture.handle ) );
	}

	if ( !texture.sampler )
	{
		GL_CHECK( glGenSamplers( 1, &texture.sampler ) );
	}

	if ( !texture.wrap )
	{
		texture.wrap = GL_CLAMP_TO_EDGE;
	}

	if ( loadFlags & Q3LOAD_TEXTURE_ANISOTROPY )
	{
		GLfloat maxSamples;
		GL_CHECK( glGetFloatv( GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxSamples ) );
		GL_CHECK( glSamplerParameterf( texture.sampler, GL_TEXTURE_MAX_ANISOTROPY_EXT, maxSamples ) );
	}

	GL_CHECK( glBindTexture( GL_TEXTURE_2D, texture.handle ) );

	GLenum minFilter;
	if ( loadFlags & Q3LOAD_TEXTURE_MIPMAP )
	{
		int maxLevels = glm::min( ( int ) glm::log2( ( float ) texture.width ), ( int ) glm::log2( ( float ) texture.height ) ); 

		int w = texture.width;
		int h = texture.height;

		for ( int mip = 0; h != 1 || w != 1; ++mip )
		{
			GL_CHECK( glTexImage2D( GL_TEXTURE_2D, 
				mip, texture.internalFormat, w, h, 0, texture.format, GL_UNSIGNED_BYTE, &texture.pixels[ 0 ] ) );

			if ( h > 1 )
			{
				h /= 2;
			}

			if ( w > 1 )
			{
				w /= 2;
			}
		}

		GL_CHECK( glGenerateMipmap( GL_TEXTURE_2D ) );
		minFilter = GL_LINEAR_MIPMAP_LINEAR;
	}
	else
	{
		GL_CHECK( glTexImage2D( GL_TEXTURE_2D, 
			0, texture.internalFormat, texture.width, texture.height, 0, texture.format, GL_UNSIGNED_BYTE, &texture.pixels[ 0 ] ) );
		minFilter = GL_LINEAR;
	}

	GL_CHECK( glSamplerParameteri( texture.sampler, GL_TEXTURE_MIN_FILTER, minFilter ) );
	GL_CHECK( glSamplerParameteri( texture.sampler, GL_TEXTURE_MAG_FILTER, GL_LINEAR ) );
	GL_CHECK( glSamplerParameteri( texture.sampler, GL_TEXTURE_WRAP_S, texture.wrap ) );
	GL_CHECK( glSamplerParameteri( texture.sampler, GL_TEXTURE_WRAP_T, texture.wrap ) );

	return true;
}

void LoadVertexLayout( uint32_t attribFlags, const Program& prog )
{
	if ( attribFlags & GLUTIL_LAYOUT_POSITION ) 
	{
		MapVec3( prog.attribs.at( "position" ), offsetof( bspVertex_t, position ) );
	}

	if ( attribFlags & GLUTIL_LAYOUT_NORMAL )
	{
		MapVec3( prog.attribs.at( "normal" ), offsetof( bspVertex_t, normal ) );
	}

	if ( attribFlags & GLUTIL_LAYOUT_COLOR )
	{
		GL_CHECK( glEnableVertexAttribArray( prog.attribs.at( "color" ) ) ); 
		GL_CHECK( glVertexAttribPointer( prog.attribs.at( "color" ), 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof( bspVertex_t ), ( void* ) offsetof( bspVertex_t, color ) ) );
	}

	if ( attribFlags & GLUTIL_LAYOUT_TEX0 )
	{
		MapAttribTexCoord( prog.attribs.at( "tex0" ), offsetof( bspVertex_t, texCoords[ 0 ] ) );
	}

	if ( attribFlags & GLUTIL_LAYOUT_LIGHTMAP )
	{
		MapAttribTexCoord( prog.attribs.at( "lightmap" ), offsetof( bspVertex_t, texCoords[ 1 ] ) );
	}
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