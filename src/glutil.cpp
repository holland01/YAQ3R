#include "glutil.h"
#include "q3bsp.h"
#include "extern/stb_image.c"

static INLINE void SetPixel( byte* dest, const byte* src, int width, int height, int bpp, int srcX, int srcY, int destX, int destY )
{
	int destOfs = ( width * destY + destX ) * bpp;
	int srcOfs = ( width * srcY + srcX ) * bpp;

	for ( int k = 0; k < bpp; ++k )
		dest[ destOfs + k ] = src[ srcOfs + k ];
}

static INLINE void FlipBytes( std::vector< byte >& out, const byte* src, int width, int height, int bpp )
{
	for ( int y = 0; y < height; ++y )
	{
		for ( int x = 0; x < width; ++x )
		{
			SetPixel( &out[ 0 ], src, width, height, bpp, x, height - y - 1, x, y );
		}
	}
}

// Assumes square dimensions; solution based off of an image rotation algorithm from "Cracking the Coding Interview"
static INLINE void RotateBytes90CCW( std::vector< byte >& image, int width, int height, int bpp )
{
	const int layerCount = height / 2;
	for ( int y = height - 1; y >= layerCount; --y )
	{
		int upMost = y;
		int downMost = height - y + 1;
		for ( int x = downMost; x < width - downMost; ++x )
		{
			int up = ( width * upMost + width - 1 - x ) * bpp; 
			int down = ( width * downMost + x ) * bpp;
			int left = ( width * x + downMost ) * bpp;
			int right = ( width * ( downMost + x ) + ( y - 1 ) ) * bpp;

			byte tmp[ 4 ] = { 0, 0, 0, 0 };
			memcpy( tmp, &image[ up ], sizeof( byte ) * bpp );
		
			memcpy( &image[ up ], &image[ left ], sizeof( byte ) * bpp );
			memcpy( &image[ left ], &image[ down ], sizeof( byte ) * bpp );
			memcpy( &image[ down ], &image[ right ], sizeof( byte ) * bpp );
			memcpy( &image[ right ], tmp, sizeof( byte ) * bpp );
		}
	}
}

bool LoadTextureFromFile( const char* texPath, GLuint texObj, GLuint samplerObj, uint32_t loadFlags, GLenum texWrap )
{
	// Load image
	// Need to also flip the image, since stbi loads pointer to upper left rather than lower left (what OpenGL expects)
	std::vector< byte > pixels;
	int width, height, bpp;
	{
		byte* imagePixels = stbi_load( texPath, &width, &height, &bpp, STBI_default );

		if ( !imagePixels )
		{
			MLOG_WARNING( "No file found for \'%s\'", texPath );
			return false;
		}
		
		pixels.resize( width * height * bpp, 0 );
		FlipBytes( pixels, imagePixels, width, height, bpp );	
		RotateBytes90CCW( pixels, width, height, bpp );

		stbi_image_free( imagePixels );
	}

	GLenum fmt;
	GLenum internalFmt;

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

	switch ( bpp )
	{
	case 1:
		fmt = GL_R;
		internalFmt = GL_R8; 
		break;
	case 3:
		internalFmt = rgb;
		fmt = GL_RGB;
		break;
	case 4:
		internalFmt = rgba;
		fmt = GL_RGBA;
		break;
	default:
		MLOG_ERROR( "Unsupported bits per pixel of %i specified; this needs to be fixed. For image file \'%s\'", bpp, texPath );
		break;
	}

	GL_CHECK( glBindTexture( GL_TEXTURE_2D, texObj ) );

	// Generate mipmaps ( if requested ) 
	int maxLevels = glm::min( ( int ) glm::log2( ( float ) width ), ( int ) glm::log2( ( float ) height ) ); 
	GLenum minFilter;
	if ( loadFlags & Q3LOAD_TEXTURE_MIPMAP )
	{
		int w = width;
		int h = height;

		for ( int mip = 0; h != 1 && w != 1; ++mip )
		{
			GL_CHECK( glTexImage2D( GL_TEXTURE_2D, mip, internalFmt, w, h, 0, fmt, GL_UNSIGNED_BYTE, &pixels[ 0 ] ) );

			if ( h > 1 )
				h /= 2;

			if ( w > 1 )
				w /= 2;
		}

		GL_CHECK( glGenerateMipmap( GL_TEXTURE_2D ) );
		minFilter = GL_LINEAR_MIPMAP_LINEAR;
	}
	else
	{
		GL_CHECK( glTexImage2D( GL_TEXTURE_2D, 0, internalFmt, width, height, 0, fmt, GL_UNSIGNED_BYTE, &pixels[ 0 ] ) );
		minFilter = GL_LINEAR;
	}

	// Configure sampler params
	if ( loadFlags & Q3LOAD_TEXTURE_ANISOTROPY )
	{
		GLfloat maxSamples;
		GL_CHECK( glGetFloatv( GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxSamples ) );
		GL_CHECK( glSamplerParameterf( samplerObj, GL_TEXTURE_MAX_ANISOTROPY_EXT, maxSamples ) );
	}

	GL_CHECK( glSamplerParameteri( samplerObj, GL_TEXTURE_MIN_FILTER, minFilter ) );
	GL_CHECK( glSamplerParameteri( samplerObj, GL_TEXTURE_MAG_FILTER, GL_LINEAR ) );
	GL_CHECK( glSamplerParameteri( samplerObj, GL_TEXTURE_WRAP_S, texWrap ) );
	GL_CHECK( glSamplerParameteri( samplerObj, GL_TEXTURE_WRAP_T, texWrap ) );

	return true;
}