#include "texture.h"
#include "glutil.h"
#include "lib/math.h"
#include <unordered_map>
#include <memory>
#include <algorithm>
#include "render_data.h"

namespace {

struct gGrid_t
{
	GLuint handle;
	uint16_t xStart, yStart;
	uint16_t xEnd, yEnd;
	float invStride;
	float invRows;
};

struct gTexture_t
{
	std::vector< gTextureImage_t > imageSlots;
	std::unordered_map<
		gTextureMakeParams_t::key_t,
		gTextureImage_t > keyMapSlots;
	glm::vec2 invRowPitch;

	bool keyMapped;
	GLenum target;
	gSamplerHandle_t sampler;
	gGrid_t* grids;
	uint8_t numGrids;

	gTexture_t( void )
		: invRowPitch( 0.0f ),
		  keyMapped( false ),
		  grids( nullptr ),
		  numGrids( 0 )
	{
	}

	void FreeGrid( gGrid_t* s )
	{
		if ( s && s->handle )
		{
			GL_CHECK( glBindTexture( target, 0 ) );
			GL_CHECK( glDeleteTextures( 1, &s->handle ) );
			s->handle = 0;
		}
	}

	const gTextureImage_t& GetSlot( uint16_t slot ) const
	{
		if ( keyMapped )
		{
			return keyMapSlots.at( ( gTextureMakeParams_t::key_t ) slot );
		}
		else
		{
			MLOG_ASSERT( slot < imageSlots.size(), "Bad index %i for texture slot", slot );
			return imageSlots[ slot ];
		}
	}

	~gTexture_t( void )
	{
		for ( uint32_t i = 0; i < numGrids; ++i )
		{
			FreeGrid( grids + i );
		}

		delete[] grids;
	}
};

using texturePointer_t = std::unique_ptr< gTexture_t >;

std::vector< texturePointer_t > gTextureMap;

gTexSlot_t gSlotStage = ( gTexSlot_t ) G_UNSPECIFIED;

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

INLINE bool ValidateTexture( const gImageParams_t& params, const gTexConfig_t& sampler )
{
	std::vector< uint8_t > zeroOut( params.width * sampler.bpp * params.height, 0 );

	GL_CHECK( glTexImage2D( GL_PROXY_TEXTURE_2D, 0,  sampler.internalFormat,
			   params.width,
			   params.height,
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
		if ( makeParams.images.size() != makeParams.keyMaps.size() )
		{
			MLOG_WARNING( "%s", "G_TEXTURE_STORAGE_KEY_MAPPED specified with entry/key size mismatch; aborting." );
			return false;
		}
	}

	return true;
}

void GenGridTexture( GLenum target,
					gGrid_t* grid,
					const gTexConfig_t& sampler,
					const uint8_t* data )
{
	GL_CHECK( glGenTextures( 1, &grid->handle ) );
	GL_CHECK( glBindTexture( target, grid->handle ) );

	GL_CHECK( glTexParameteri( target, GL_TEXTURE_WRAP_S, sampler.wrap ) );
	GL_CHECK( glTexParameteri( target, GL_TEXTURE_WRAP_T, sampler.wrap ) );
	GL_CHECK( glTexParameteri( target, GL_TEXTURE_MAG_FILTER, sampler.magFilter ) );
	GL_CHECK( glTexParameteri( target, GL_TEXTURE_MIN_FILTER, sampler.minFilter) );

	GL_CHECK( glTexImage2D( GL_TEXTURE_2D, 0, sampler.internalFormat,
			grid->xEnd - grid->xStart,
			grid->yEnd - grid->yStart,
			0,
			sampler.format,
			GL_UNSIGNED_BYTE, data ) );

	GL_CHECK( glBindTexture( target, 0 ) );
}

bool GenTextureData( gTexture_t* tt, const gImageParams_t& params )
{
	if ( !tt )
	{
		return false;
	}

	const gTexConfig_t& sampler = GetTexConfig( params.sampler );

	{
		gImageParams_t subdivision;
		subdivision.width = params.width;
		subdivision.height = params.height;
		subdivision.data = std::move( params.data );

		GLint gpuMaxDims;
		GL_CHECK( glGetIntegerv( GL_MAX_TEXTURE_SIZE, &gpuMaxDims ) );

		uint32_t xDivide = 0, yDivide = 0;

		while ( !ValidateTexture( subdivision, sampler ) )
		{
			if ( subdivision.width > gpuMaxDims )
			{
				subdivision.width >>= 1;
				xDivide++;
			}

			if ( subdivision.height > gpuMaxDims )
			{
				subdivision.height >>= 1;
				yDivide++;
			}
		}

		tt->numGrids = 1 << ( xDivide + yDivide );
		tt->grids = new gGrid_t[ tt->numGrids ]();
		tt->grids[ 0 ].xStart = tt->grids[ 0 ].yStart = 0;
		tt->grids[ 0 ].xEnd = params.width;
		tt->grids[ 0 ].yEnd = params.height;
	}

	GenGridTexture( tt->target, tt->grids, sampler, &params.data[ 0 ] );

	return true;
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
		gDummy->sampler.id = 0;

		GenTextureData( gDummy.get(), params );

		gTextureImage_t data;
		data.dims = glm::vec2( ( float ) params.width, ( float ) params.height );
		data.imageScaleRatio = glm::vec2( 1.0f, 1.0f );
		data.stOffsetEnd = glm::vec2( 1.0f, 1.0f );
		data.stOffsetStart = glm::vec2( 0.0f, 0.0f );

		gDummy->imageSlots.push_back( data );

		gDummy->invRowPitch = glm::vec2( 1.0f, 1.0f );
	}
}

INLINE gGrid_t* GridFromSlot( gTextureHandle_t handle, uint32_t slotIndex )
{
	if ( slotIndex == G_UNSPECIFIED )
	{
		return nullptr;
	}

	const gTexture_t* t = GetTexture( handle );
	const gTextureImage_t& data = GTextureImage( handle, slotIndex );

	// TODO: If this slot has a good relationship with the layout of the
	// grids, then we can make this more intelligent. (geometrically,
	// the slot should map to rectangle in a grid of grids - each grid
	// maps to its own set of x,y ranges; the slot is likely to have a corresponding
	// relationship with these x,y ranges.)
	for ( uint8_t i = 0; i < t->numGrids; ++i )
	{
		if ( data.stOffsetStart.x < t->grids[ i ].xStart ) continue;
		if ( data.stOffsetEnd.x > t->grids[ i ].xEnd ) continue;

		if ( data.stOffsetStart.y < t->grids[ i ].yStart ) continue;
		if ( data.stOffsetEnd.y > t->grids[ i ].yEnd ) continue;

		return t->grids + i;
	}

	return nullptr;
}

void CalcGridDimensions( gImageParams_t& canvasParams,
						 gImageParams_t& slotParams,
						 const gTextureMakeParams_t& makeParams,
						 gImageParamList_t& images )
{
	glm::ivec2 maxDims( 0 );

	std::sort( images.begin(), images.end(), [ &maxDims ]( gImageParams_t& a, gImageParams_t& b )
	{
		if ( a.width > maxDims.x )
			maxDims.x = a.width;

		if ( a.height > maxDims.y )
			maxDims.y = a.height;

		float alength = glm::length( glm::vec2( ( float ) a.width, ( float ) a.height ) );
		float blength = glm::length( glm::vec2( ( float ) b.width, ( float ) b.height ) );

		return alength < blength;
	});

	maxDims.x = NextPower2( maxDims.x );
	maxDims.y = NextPower2( maxDims.y );

	uint32_t arrayDims = NextSquare( images.size() );

	canvasParams.sampler = makeParams.sampler;
	GSetImageBuffer( canvasParams, maxDims.x * arrayDims, maxDims.y * arrayDims, 0 );

	slotParams.width = maxDims.x;
	slotParams.height = maxDims.y;
}

void GenSubdivision( gTexture_t* tt,
					 uint32_t grid,
					 gTextureMakeParams_t& makeParams,
					 const gImageParams_t& canvasParams,
					 const gImageParams_t& slotParams,
					 gImageParams_t* base,
					 gImageParams_t* start,
					 gImageParams_t* bound )
{
	const uint32_t stride = uint32_t( canvasParams.width / slotParams.width );
	const uint32_t rows = uint32_t( canvasParams.height / slotParams.height );

	float invSlotWidth = 1.0f / ( float ) slotParams.width;
	float invSlotHeight = 1.0f / ( float ) slotParams.height;

	float invStride = 1.0f / ( float ) stride;
	float invRows = 1.0f / ( float ) rows;

	tt->grids[ grid ].invStride = invStride;
	tt->grids[ grid ].invRows = invRows;

	const gTexConfig_t& sampler = GetTexConfig( makeParams.sampler );

	GL_CHECK( glBindTexture( tt->target, tt->grids[ grid ].handle ) );

	uint32_t x = 0, y = 0;

	for ( gImageParams_t* iImage = start; iImage != bound; ++iImage )
	{
		gImageParams_t& image = *iImage;

		uint32_t yb = y * slotParams.height;
		uint32_t xb = x * slotParams.width;

		GL_CHECK( glTexSubImage2D( tt->target, 0, xb, yb, image.width,
			image.height, sampler.format, GL_UNSIGNED_BYTE, &image.data[ 0 ] ) );

		float fxStart = ( float )xb * invSlotWidth * invStride;
		float fyStart = ( float )yb * invSlotHeight * invRows;

		float fxEnd = fxStart + ( float )image.width * invSlotWidth * invStride;
		float fyEnd = fyStart + ( float )image.height * invSlotHeight * invRows;

		gTextureImage_t data;

		data.stOffsetStart = glm::vec2( fxStart, fyStart );
		data.stOffsetEnd = glm::vec2( fxEnd, fyEnd );
		data.imageScaleRatio =
				glm::vec2( ( float ) image.width * invSlotWidth, ( float ) image.height * invSlotHeight );
		data.dims = glm::vec2( ( float ) image.width, ( float ) image.height );

		uintptr_t slot = ( uintptr_t )( iImage - base );

		if ( tt->keyMapped )
		{
			tt->keyMapSlots[ makeParams.keyMaps[ ( gTextureMakeParams_t::key_t ) slot ] ] = data;
		}
		else
		{
			tt->imageSlots[ slot ] = data;
		}

		uint32_t next = ( x + 1 ) % rows;

		if ( next == 0 )
			y++;

		image.data.clear();
		x = next;
	}

	GL_CHECK( glBindTexture( tt->target, 0 ) );
}

gTexture_t* MakeTexture( gTextureMakeParams_t& makeParams )
{
	gImageParams_t canvasParams, slotParams;

	CalcGridDimensions( canvasParams, slotParams, makeParams, makeParams.images );

	gTexture_t* tt = new gTexture_t();
	tt->target = GL_TEXTURE_2D;
	tt->keyMapped = !!( makeParams.flags & G_TEXTURE_STORAGE_KEY_MAPPED_BIT );

	if ( !tt->keyMapped )
	{
		tt->imageSlots.resize( ( canvasParams.width / slotParams.width )
							  * ( canvasParams.height / slotParams.height ) );
	}

	GenTextureData( tt, canvasParams );

	for ( uint32_t i = 0; i < tt->numGrids; ++i )
	{
		GenSubdivision( tt, i,
					   makeParams,
					   canvasParams, slotParams,
					   &makeParams.images[ 0 ],
					   &makeParams.images[ i ],
					   &makeParams.images[ makeParams.images.size() - 1 ] );
	}

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

	gTexture_t* texture = MakeTexture( makeParams );

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

void GStageSlot( gTexSlot_t slot )
{
	gSlotStage = slot;
}

void GUnstageSlot( void )
{
	gSlotStage = (gTexSlot_t ) G_UNSPECIFIED;
}

void GBindTexture( const gTextureHandle_t& handle, uint32_t offset )
{
	const gTexture_t* t = GetTexture( handle );

	if ( !t )
	{
		return;
	}

	gGrid_t* grid = nullptr;

	if ( t->numGrids == 1 )
	{
		grid = t->grids;
	}
	else
	{
		grid = GridFromSlot( handle, gSlotStage );
	}

	if ( grid )
	{
		GL_CHECK( glActiveTexture( GL_TEXTURE0 + offset ) );
		GL_CHECK( glBindTexture( t->target, grid->handle ) );
	}
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

	return t->GetSlot( slot );
}

const gTextureImage_t& GTextureImage( const gTextureHandle_t& handle )
{
	MLOG_ASSERT( !G_VNULL( gSlotStage ), "Not slot stage set. Handle: %i", handle.id );

	return GTextureImage( handle, gSlotStage );
}

glm::vec2 GTextureInverseRowPitch( const gTextureHandle_t& handle )
{
	assert( handle.id < gTextureMap.size() );

	MLOG_ASSERT( !G_VNULL( gSlotStage ), "No slot stage lroaded for handle: %i", handle.id );

	gGrid_t* grid = GridFromSlot( handle, gSlotStage );

	return glm::vec2( grid->invStride, grid->invRows );
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
