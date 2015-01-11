#include "glutil.h"
#include "q3bsp.h"
#include "log.h"
#include "extern/stb_image.c"

bool LoadTextureFromFile( const char* texPath, GLuint texObj, GLuint samplerObj, uint32_t loadFlags, GLenum texWrap )
{
	// Load image
	int width, height, bpp;
	byte* imagePixels = stbi_load( texPath, &width, &height, &bpp, STBI_default );

	if ( !imagePixels )
	{
		MLOG_WARNING( "No file found for \'%s\'", texPath );
		return false;
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
			GL_CHECK( glTexImage2D( GL_TEXTURE_2D, mip, internalFmt, w, h, 0, fmt, GL_UNSIGNED_BYTE, imagePixels ) );

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
		GL_CHECK( glTexImage2D( GL_TEXTURE_2D, 0, internalFmt, width, height, 0, fmt, GL_UNSIGNED_BYTE, imagePixels ) );
		minFilter = GL_LINEAR;
	}
	
	stbi_image_free( imagePixels );

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