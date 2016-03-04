#include "texture.h"
#include "glutil.h"
#include <unordered_map>
#include <memory>
#include <algorithm>
#include "render_data.h"

namespace {

using imageSlotKeyMap_t = std::unordered_map< uint32_t, gTextureImage_t* >;

struct gTexture_t
{
	bool srgb = false;
	bool mipmap = false;

	uint32_t id = 0;
	uint32_t samplerID = 0;

	GLuint handle = 0;
	GLuint maxMip = 0;
	GLsizei width = 0;
	GLsizei height = 0;
	GLsizei bpp = 0; // bpp is in bytes
	GLenum target;

	std::string name;

	std::vector< gTextureImage_t > texCoordSlots;

	imageSlotKeyMap_t keyedSlots;

	glm::vec2 invRowPitch;

	~gTexture_t( void )
	{
		if ( handle )
			GL_CHECK( glDeleteTextures( 1, &handle ) );
	}
};

using texturePointer_t = std::unique_ptr< gTexture_t >;

std::vector< texturePointer_t > gTextureMap;


struct gTexConfig_t
{
	bool mipmap;
	int8_t bpp;
	uint32_t wrap;
	uint32_t minFilter;
	uint32_t magFilter;
	uint32_t format;
	uint32_t internalFormat;
};

std::vector< gTexConfig_t > gSamplers;

INLINE bool ValidateTexture( const gTexture_t& tt, const gTexConfig_t& sampler )
{
	std::vector< uint8_t > zeroOut( tt.width * sampler.bpp * tt.height, 0 );

	GL_CHECK( glTexImage2D( GL_PROXY_TEXTURE_2D, 0,  sampler.internalFormat,
			   tt.width,
			   tt.height,
			   0,
			   sampler.format,
			   GL_UNSIGNED_BYTE, &zeroOut[ 0 ] ) );

	GLint testWidth, testHeight;
	GL_CHECK( glGetTexLevelParameteriv( GL_PROXY_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &testWidth ) );
	GL_CHECK( glGetTexLevelParameteriv( GL_PROXY_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &testHeight ) );
	
	return testWidth && testHeight;
}


INLINE gTexture_t* MakeTexture_GLES( const gImageParams_t& canvasParams,
						 const gImageParams_t& slotParams,
						 gTextureMakeParams_t& makeParams )
{
	gTexture_t* tt = new gTexture_t();
	tt->width = canvasParams.width;
	tt->height = canvasParams.height;

	gTexConfig_t& sampler = gSamplers[ makeParams.sampler.id ];

	tt->target = GL_TEXTURE_2D;

	/*
	if ( !ValidateTexture( *tt, sampler ) )
	{
		MLOG_WARNING( "Invalid texture load of texels %i x %i was attempted; bailing.", tt->width, tt->height );
		goto done;
	}
	*/

	GL_CHECK( glGenTextures( 1, &tt->handle ) );
	GL_CHECK( glBindTexture( tt->target, tt->handle ) );
	GL_CHECK( glTexParameteri( tt->target, GL_TEXTURE_WRAP_S, sampler.wrap ) );
	GL_CHECK( glTexParameteri( tt->target, GL_TEXTURE_WRAP_T, sampler.wrap ) );
	GL_CHECK( glTexParameteri( tt->target, GL_TEXTURE_MAG_FILTER, sampler.magFilter ) );
	GL_CHECK( glTexParameteri( tt->target, GL_TEXTURE_MIN_FILTER, sampler.minFilter) );

	{
		std::vector< uint8_t > zeroOut( canvasParams.width * sampler.bpp * canvasParams.height, 0 );
		GL_CHECK( glTexImage2D( GL_TEXTURE_2D, 0, sampler.internalFormat,
			   canvasParams.width,
			   canvasParams.height,
			   0,
			   sampler.format,
			   GL_UNSIGNED_BYTE, &zeroOut[ 0 ] ) );
	}

	const uint32_t stride = uint32_t( canvasParams.width / slotParams.width );
	const uint32_t rows = uint32_t( canvasParams.height / slotParams.height );

	tt->texCoordSlots.resize( rows * stride );

	float invSlotWidth = 1.0f / ( float ) slotParams.width;
	float invSlotHeight = 1.0f / ( float ) slotParams.height;

	tt->invRowPitch.x = 1.0f / ( float ) stride;
	tt->invRowPitch.y = 1.0f / ( float ) rows;

	GLint param;
	GL_CHECK( glGetIntegerv( GL_MAX_RECTANGLE_TEXTURE_SIZE, &param ) );
	UNUSED( param );

	uint32_t y = 0, x = 0;
	for ( auto iImage = makeParams.start; iImage != makeParams.end; ++iImage )
	{
		gImageParams_t& image = *iImage;

		uint32_t yb = y * slotParams.height;
		uint32_t xb = ( x % stride ) * slotParams.width;

		GL_CHECK( glTexSubImage2D( GL_TEXTURE_2D, 0, xb, yb, image.width,
			image.height, sampler.format, GL_UNSIGNED_BYTE, &image.data[ 0 ] ) );

		// !FIXME?: invSlotWidth may be able to be removed from these computations - it would be good
		// to double check these on paper
		float fxStart = ( float )xb * invSlotWidth * tt->invRowPitch.x;
		float fyStart = ( float )yb * invSlotHeight * tt->invRowPitch.y;

		float fxEnd = fxStart + ( float )image.width * invSlotWidth * tt->invRowPitch.x;
		float fyEnd = fyStart + ( float )image.height * invSlotHeight * tt->invRowPitch.y;

		uint32_t slot = y * stride + ( x % stride );

		tt->texCoordSlots[ slot ].stOffsetStart = glm::vec2( fxStart, fyStart );
		tt->texCoordSlots[ slot ].stOffsetEnd = glm::vec2( fxEnd, fyEnd );
		tt->texCoordSlots[ slot ].imageScaleRatio =
				glm::vec2( ( float ) image.width * invSlotWidth, ( float ) image.height * invSlotHeight );
		tt->texCoordSlots[ slot ].dims = glm::vec2( ( float ) image.width, ( float ) image.height );

		if ( ( xb + slotParams.width ) % canvasParams.width == 0 )
			y++;

		image.data.clear();
		x++;
	}

done:
	GL_CHECK( glBindTexture( tt->target, 0 ) );
	return tt;
}

bool DetermineImageFormats( int8_t bpp, uint32_t& format, uint32_t& internalFormat )
{
	switch( bpp )
	{
	case 1:
		format = G_BYTE_FORMAT;
		internalFormat = G_INTERNAL_BYTE_FORMAT;
		break;

	case 4:
		format = G_RGBA_FORMAT;
		internalFormat = G_INTERNAL_RGBA_FORMAT;
		break;
	default:
		MLOG_WARNING( "Unsupported image format of %i.", bpp );
		return false;
		break;
	}

	return true;
}

} // end namespace


gSamplerHandle_t GMakeSampler(
	int8_t bpp,
	bool mipmapped,
	uint32_t wrap
)
{
	gSamplerHandle_t sampler;

	uint32_t format, internalFormat;
	if ( !DetermineImageFormats( bpp, format, internalFormat ) )
	{
		sampler.id = G_UNSPECIFIED;
		return sampler;
	}

	uint32_t minFilter;
	if ( mipmapped )
		minFilter = GL_LINEAR_MIPMAP_LINEAR;
	else
		minFilter = G_MAG_FILTER;

	gTexConfig_t conf =
	{
		mipmapped,
		bpp,
		wrap,
		minFilter,
		G_MAG_FILTER,
		format,
		internalFormat
	};

	sampler.id = gSamplers.size();
	gSamplers.push_back( conf );

	return sampler;
}

int8_t GSamplerBPP( const gSamplerHandle_t& sampler )
{
 	if ( sampler.id < gSamplers.size() )
	{
		return gSamplers[ sampler.id ].bpp;
	}

	return 0;
}

gTextureHandle_t GMakeTexture( gTextureMakeParams_t& makeParams, uint32_t flags )
{
	UNUSED( flags );

	glm::ivec2 maxDims( 0 );

	std::for_each( makeParams.start, makeParams.end, [ &maxDims ]( const gImageParams_t& img )
	{
		if ( img.width > maxDims.x )
			maxDims.x = img.width;

		if ( img.height > maxDims.y )
			maxDims.y = img.height;
	});

	maxDims.x = int32_t( glm::pow( 2.0f, glm::ceil( glm::log2( ( float ) maxDims.x ) ) ) );
	maxDims.y = int32_t( glm::pow( 2.0f, glm::ceil( glm::log2( ( float ) maxDims.y ) ) ) );

	//maxDims.x = glm::max( maxDims.x, maxDims.y );
	//maxDims.y = glm::max( maxDims.x, maxDims.y );

	size_t numImages = ( size_t )( makeParams.end - makeParams.start );

	uint32_t closeSquare = ( uint32_t )( glm::pow( 2.0f, glm::ceil( glm::log2( ( float ) numImages ) ) ) );
	uint32_t arrayDims = 2;

	while ( arrayDims * arrayDims < closeSquare )
		arrayDims += 2;

	// TODO: just make these extra imageParams ivec2 when passing to the make texture function...
	gImageParams_t canvasParams;
	canvasParams.width = maxDims.x * arrayDims;
	canvasParams.height = maxDims.y * arrayDims;

	gImageParams_t slotParams;
	slotParams.width = maxDims.x;
	slotParams.height = maxDims.y;

	gTexture_t* texture = MakeTexture_GLES( canvasParams, slotParams, makeParams );

	gTextureHandle_t handle =
	{
		( uint32_t ) gTextureMap.size()
	};

	gTextureMap.push_back( texturePointer_t( texture ) );

	return handle;
}

void GFreeTexture( gTextureHandle_t& handle )
{
	if ( handle.id < gTextureMap.size() )
	{
		gTextureMap.erase( gTextureMap.begin() + handle.id );
	}

	handle.id = G_UNSPECIFIED;
}

void GBindTexture( const gTextureHandle_t& handle, uint32_t offset )
{
	const gTexture_t* t = gTextureMap[ handle.id ].get();

	GL_CHECK( glActiveTexture( GL_TEXTURE0 + offset ) );
	GL_CHECK( glBindTexture( t->target, t->handle ) );
}

void GReleaseTexture( const gTextureHandle_t& handle, uint32_t offset )
{
	const gTexture_t* t = gTextureMap[ handle.id ].get();

	GL_CHECK( glActiveTexture( GL_TEXTURE0 + offset ) );
	GL_CHECK( glBindTexture( t->target, 0 ) );
}

const gTextureImage_t& GTextureImage( const gTextureHandle_t& handle, uint32_t slot )
{
	assert( handle.id < gTextureMap.size() );

	const gTexture_t* t = gTextureMap[ handle.id ].get();
	assert( slot < t->texCoordSlots.size() );

	return t->texCoordSlots[ slot ];
}

const gTextureImage_t& GTextureImageByKey( const gTextureHandle_t& handle, uint32_t key )
{
	assert( handle.id < gTextureMap.size() );

	const gTexture_t* t = gTextureMap[ handle.id ].get();

	return *( t->keyedSlots.at( key ) );
}

glm::vec2 GTextureInverseRowPitch( const gTextureHandle_t& handle )
{
	assert( handle.id < gTextureMap.size() );

	return gTextureMap[ handle.id ]->invRowPitch;
}

bool GSetImageBuffer( gImageParams_t& image, int32_t width, int32_t height, uint8_t fillValue )
{
	if ( image.sampler.id == G_UNSPECIFIED ) return false;
	if ( width < 0 ) return false;
	if ( height < 0 ) return false;

	image.width = width;
	image.height = height;
	image.data.resize( width * height * gSamplers[ image.sampler.id ].bpp, fillValue );

	return true;
}

void GSetAlignedImageData( gImageParams_t& destImage, 
						    uint8_t* sourceData, 
					        int8_t sourceBPP, 
						    uint32_t numPixels,
					        uint8_t fetchChannel )
{
	switch ( GSamplerBPP( destImage.sampler ) )
	{
	case 1:
		Pixels_ToR( &destImage.data[ 0 ], sourceData, sourceBPP, fetchChannel, numPixels );
		break;

	default:
		Pixels_To32Bit( &destImage.data[ 0 ], sourceData, sourceBPP, numPixels );
		break;
	}
}

bool GLoadImageFromFile( const std::string& imagePath, gImageParams_t& image )
{
	int32_t width, height, bpp;

	if ( image.sampler.id == G_UNSPECIFIED )
	{
		MLOG_WARNING( "image passed that\'s missing a sampler" );
		return false;
	}

	std::vector< uint8_t > tmp;
	if ( !File_GetPixels( imagePath, tmp, bpp, width, height ) )
		return false;

	if ( bpp != gSamplers[ image.sampler.id ].bpp )
	{
		uint32_t numPixels = width * height;
		image.data.resize( numPixels * gSamplers[ image.sampler.id ].bpp, 255 );
		GSetAlignedImageData( image, &tmp[ 0 ], gSamplers[ image.sampler.id ].bpp, numPixels );
	}
	else
	{
		image.data = std::move( tmp );
	}

	image.width = width;
	image.height = height;

	return true;
}
