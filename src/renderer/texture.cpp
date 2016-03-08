#include "texture.h"
#include "glutil.h"
#include "lib/math.h"
#include <unordered_map>
#include <memory>
#include <algorithm>
#include "render_data.h"

namespace {

struct gTexture_t
{
	bool srgb = false;
	bool mipmap = false;
	bool keyMapped = false;

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
	
	std::unordered_map< 
		gTextureMakeParams_t::key_t, 
		gTextureImage_t > keyMapSlots;

	glm::vec2 invRowPitch;

	~gTexture_t( void )
	{
		if ( handle )
			GL_CHECK( glDeleteTextures( 1, &handle ) );
	}
};

using texturePointer_t = std::unique_ptr< gTexture_t >;

std::vector< texturePointer_t > gTextureMap;

std::unique_ptr< gTexture_t > gDummy( nullptr );

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

INLINE const gTexConfig_t& GetTexConfig( gSamplerHandle_t handle )
{
	MLOG_ASSERT( handle.id < gSamplers.size(), 
				"Attempt to access nonexistant sampler! ID: %iu, size: %iu", 
				handle.id, ( uint32_t )gSamplers.size() );

	return gSamplers[ handle.id ];
}

INLINE const gTexture_t* GetTexture( gTextureHandle_t handle )
{
	if ( handle.id >= gTextureMap.size() )
	{
		return gDummy.get();
	}
	else
	{
		return gTextureMap[ handle.id ].get();;
	}
}

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

INLINE bool ValidateMakeParams( const gTextureMakeParams_t& makeParams )
{
	if ( !!( makeParams.flags & G_TEXTURE_STORAGE_KEY_MAPPED_BIT ) )
	{
		size_t numEntries = ( size_t )( makeParams.end - makeParams.start );
		
		if ( numEntries != makeParams.keyMaps.size() )
		{
			MLOG_WARNING( "%s", "G_TEXTURE_STORAGE_KEY_MAPPED specified with entry/key size mismatch; aborting." );
			return false;
		}
	}

	return true;
}

void GenTextureData( gTexture_t* tt, const gImageParams_t& params )
{
	if ( !tt )
	{
		return;
	}

	const gTexConfig_t& sampler = GetTexConfig( params.sampler );

	tt->width = params.width;
	tt->height = params.height;

	GL_CHECK( glGenTextures( 1, &tt->handle ) );
	GL_CHECK( glBindTexture( tt->target, tt->handle ) );

	GL_CHECK( glTexParameteri( tt->target, GL_TEXTURE_WRAP_S, sampler.wrap ) );
	GL_CHECK( glTexParameteri( tt->target, GL_TEXTURE_WRAP_T, sampler.wrap ) );
	GL_CHECK( glTexParameteri( tt->target, GL_TEXTURE_MAG_FILTER, sampler.magFilter ) );
	GL_CHECK( glTexParameteri( tt->target, GL_TEXTURE_MIN_FILTER, sampler.minFilter) );

	GL_CHECK( glTexImage2D( GL_TEXTURE_2D, 0, sampler.internalFormat,
			params.width,
			params.height,
			0,
			sampler.format,
			GL_UNSIGNED_BYTE, &params.data[ 0 ] ) );

	GL_CHECK( glBindTexture( tt->target, 0 ) );
}

INLINE void TryAllocDummy( void )
{
	if ( !gDummy && !gSamplers.empty() ) // The sampler we use in particular is pretty much irrelevant
 	{
		gImageParams_t params;
		params.sampler.id = 0;

		GSetImageBuffer( params, 16, 16, 0 );

		gDummy.reset( new gTexture_t() );
		gDummy->target = GL_TEXTURE_2D;
		gDummy->samplerID = 0;

		GenTextureData( gDummy.get(), params );

		gTextureImage_t data;
		data.dims = glm::vec2( ( float ) gDummy->width, ( float ) gDummy->height );
		data.imageScaleRatio = glm::vec2( 1.0f, 1.0f );
		data.stOffsetEnd = glm::vec2( 1.0f, 1.0f );
		data.stOffsetStart = glm::vec2( 0.0f, 0.0f );

		gDummy->texCoordSlots.push_back( data );

		gDummy->invRowPitch = glm::vec2( 1.0f, 1.0f );
	}
}

gTexture_t* MakeTexture( const gImageParams_t& canvasParams,
						 const gImageParams_t& slotParams,
						 gTextureMakeParams_t& makeParams )
{
	gTexture_t* tt = new gTexture_t();
	tt->target = GL_TEXTURE_2D;
	tt->keyMapped = !!( makeParams.flags & G_TEXTURE_STORAGE_KEY_MAPPED_BIT );

	GenTextureData( tt, canvasParams );
	
	const uint32_t stride = uint32_t( canvasParams.width / slotParams.width );
	const uint32_t rows = uint32_t( canvasParams.height / slotParams.height );

	if ( !tt->keyMapped )
	{
		tt->texCoordSlots.resize( rows * stride );
	}

	float invSlotWidth = 1.0f / ( float ) slotParams.width;
	float invSlotHeight = 1.0f / ( float ) slotParams.height;

	tt->invRowPitch.x = 1.0f / ( float ) stride;
	tt->invRowPitch.y = 1.0f / ( float ) rows;

	uint32_t y = 0, x = 0;

	const gTexConfig_t& sampler = GetTexConfig( makeParams.sampler );

	GL_CHECK( glBindTexture( tt->target, tt->handle ) );

	for ( auto iImage = makeParams.start; iImage != makeParams.end; ++iImage )
	{
		gImageParams_t& image = *iImage;

		uint32_t yb = y * slotParams.height;
		uint32_t xb = ( x % stride ) * slotParams.width;

		GL_CHECK( glTexSubImage2D( tt->target, 0, xb, yb, image.width,
			image.height, sampler.format, GL_UNSIGNED_BYTE, &image.data[ 0 ] ) );

		// !FIXME?: invSlotWidth may be able to be removed from these computations - it would be good
		// to double check these on paper
		float fxStart = ( float )xb * invSlotWidth * tt->invRowPitch.x;
		float fyStart = ( float )yb * invSlotHeight * tt->invRowPitch.y;

		float fxEnd = fxStart + ( float )image.width * invSlotWidth * tt->invRowPitch.x;
		float fyEnd = fyStart + ( float )image.height * invSlotHeight * tt->invRowPitch.y;

		gTextureImage_t data;

		data.stOffsetStart = glm::vec2( fxStart, fyStart );
		data.stOffsetEnd = glm::vec2( fxEnd, fyEnd );
		data.imageScaleRatio =
				glm::vec2( ( float ) image.width * invSlotWidth, ( float ) image.height * invSlotHeight );
		data.dims = glm::vec2( ( float ) image.width, ( float ) image.height );

#ifdef DEBUG
		{
			// It's good to clarify the difference between the iterator-based indexing and
			// our grid (x,y) indexing: there's always the chance that we may
			// want to modify the actual layout of the textures such that there's no guarantee
			// both indexing schemes are equivalent.

			// As it stands, the only reason why slot == index at the time of this writing
			// is because any extra slots which are added (for the sake of conforming to power of two size)
			// will never be filled, given the linear appending.

			size_t index = ( size_t )( iImage - makeParams.start );
			uint32_t slot = ( x % stride ) + y * stride;
			assert( index == slot );
		}
#endif

		if ( tt->keyMapped )
		{
			size_t index = ( size_t )( iImage - makeParams.start );
			tt->keyMapSlots[ makeParams.keyMaps[ ( gTextureMakeParams_t::key_t ) index ] ] = data;
		}
		else
		{
			uint32_t slot = ( x % stride ) + y * stride;
			tt->texCoordSlots[ slot ] = data;
		}

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

gTextureHandle_t GMakeTexture( gTextureMakeParams_t& makeParams )
{
	TryAllocDummy();

	if ( !ValidateMakeParams( makeParams ) )
	{
		return { G_UNSPECIFIED };
	}

	glm::ivec2 maxDims( 0 );

	std::for_each( makeParams.start, makeParams.end, [ &maxDims ]( const gImageParams_t& img )
	{
		if ( img.width > maxDims.x )
			maxDims.x = img.width;

		if ( img.height > maxDims.y )
			maxDims.y = img.height;
	});

	maxDims.x = NextPower2( maxDims.x );
	maxDims.y = NextPower2( maxDims.y );

	size_t numImages = ( size_t )( makeParams.end - makeParams.start );

	uint32_t closeSquare = NextPower2( numImages );
	uint32_t arrayDims = 2;

	while ( arrayDims * arrayDims < closeSquare )
		arrayDims += 2;

	gImageParams_t canvasParams;
	canvasParams.sampler = makeParams.sampler;
	GSetImageBuffer( canvasParams, maxDims.x * arrayDims, maxDims.y * arrayDims, 0 );

	gImageParams_t slotParams;
	slotParams.width = maxDims.x;
	slotParams.height = maxDims.y;

	gTexture_t* texture = MakeTexture( canvasParams, slotParams, makeParams );

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
	const gTexture_t* t = GetTexture( handle );
	GL_CHECK( glActiveTexture( GL_TEXTURE0 + offset ) );
	GL_CHECK( glBindTexture( t->target, t->handle ) );
}

void GReleaseTexture( const gTextureHandle_t& handle, uint32_t offset )
{
	const gTexture_t* t = GetTexture( handle );
	GL_CHECK( glActiveTexture( GL_TEXTURE0 + offset ) );
	GL_CHECK( glBindTexture( t->target, 0 ) );
}

const gTextureImage_t& GTextureImage( const gTextureHandle_t& handle, uint32_t slot )
{
	assert( handle.id < gTextureMap.size() );

	const gTexture_t* t = GetTexture( handle );
	
	// Ensure we don't have something like a negative texture index
	if ( t == gDummy.get() )
	{
		slot = 0;
	}

	if ( t->keyMapped )
	{
		return t->keyMapSlots.at( ( gTextureMakeParams_t::key_t ) slot );
	}
	else
	{
		MLOG_ASSERT( slot < t->texCoordSlots.size(), "Bad index %i for texture slot", slot );
		return t->texCoordSlots[ slot ];
	}
}

glm::vec2 GTextureInverseRowPitch( const gTextureHandle_t& handle )
{
	assert( handle.id < gTextureMap.size() );

	return gTextureMap[ handle.id ]->invRowPitch;
}

bool GSetImageBuffer( gImageParams_t& image, int32_t width, int32_t height, uint8_t fillValue )
{
	if ( width < 0 ) return false;
	if ( height < 0 ) return false;

	const gTexConfig_t& sampler = GetTexConfig( image.sampler );

	image.width = width;
	image.height = height;
	image.data.resize( width * height * sampler.bpp, fillValue );

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
		GSetAlignedImageData( image, &tmp[ 0 ], bpp, numPixels );
	}
	else
	{
		image.data = std::move( tmp );
	}

	image.width = width;
	image.height = height;

	return true;
}