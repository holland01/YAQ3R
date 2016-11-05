#include "texture.h"
#include "glutil.h"
#include "lib/math.h"
#include "lib/atlas_gen.h"
#include <memory>
#include <algorithm>
#include "render_data.h"
#include <unordered_map>
#include <unordered_set>

namespace {

struct gGrid_t
{
	GLuint handle;
	uint16_t xStart, yStart;
	uint16_t xEnd, yEnd;
	float invStride;
	float invPitch;
};

using gTextureKeySlotMap_t = std::unordered_map< gTextureImageKey_t,
	gTextureImage_t >;

struct gTexture_t;

std::unique_ptr< gTexture_t > gDummy( nullptr );

struct gTexture_t
{
	std::vector< gTextureImage_t > imageSlots;
	gTextureKeySlotMap_t keyMapSlots;
	glm::vec2 invRowPitch;

	std::vector< glm::ivec2 > dimensions;

	bool keyMapped;

	GLenum target;
	glm::ivec2 megaDims;

	gGrid_t* grids;
	gSamplerHandle_t sampler;

	uint8_t numGrids;

	gTexture_t( void )
		: invRowPitch( 0.0f ),
		  keyMapped( false ),
		  megaDims( 0 ),
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
			auto keyed = keyMapSlots.find( ( gTextureImageKey_t ) slot );

			if ( keyed != keyMapSlots.end() )
			{
				return keyed->second;
			}

			return gDummy->imageSlots[ 0 ];

			//return keyMapSlots.at( ( gTextureImageKey_t ) slot );
		}
		else
		{
			MLOG_ASSERT( slot < imageSlots.size(),
				"Bad index %i for texture slot", slot );

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
		MLOG_WARNING(
			"Bad texture handle. ID received is %i. Returning dummy instead.",
			handle.id );

		return gDummy.get();
	}
	else
	{
		return gTextureMap[ handle.id ].get();
	}
}

INLINE bool ValidateTexture( const gImageParams_t& params,
	const gTexConfig_t& sampler )
{
	UNUSED( sampler );
	GLint maxSize;
	GL_CHECK( glGetIntegerv( GL_MAX_TEXTURE_SIZE, &maxSize ) );

	return params.width <= maxSize && params.height <= maxSize;
}

INLINE bool ValidateMakeParams( const gTextureMakeParams_t& makeParams )
{
	if ( !!( makeParams.flags & G_TEXTURE_STORAGE_KEY_MAPPED_BIT ) )
	{
		if ( makeParams.images.size() != makeParams.keyMaps.size() )
		{
			MLOG_WARNING( "%s", "G_TEXTURE_STORAGE_KEY_MAPPED specified "\
				" with entry/key size mismatch; aborting." );
			return false;
		}
	}

	return true;
}

INLINE bool InRange( const gGrid_t& grid,
 	const glm::vec2& origin, const gImageParams_t& image )
{
	glm::vec2 gridOrigin( grid.xStart, grid.yStart );
	glm::vec2 gridBound( grid.xEnd, grid.yEnd );
	glm::vec2 mapBound( origin.x + image.width,
		origin.y + image.height );

	return glm::all( glm::lessThanEqual( gridOrigin, origin ) )
		&& glm::all( glm::lessThan( mapBound, gridBound ) );
}

void GenGridTexture( GLenum target,
					gGrid_t& grid,
					const gTexConfig_t& sampler,
					const uint8_t* data )
{
	GL_CHECK( glGenTextures( 1, &grid.handle ) );
	GL_CHECK( glBindTexture( target, grid.handle ) );

	GL_CHECK( glTexParameteri( target, GL_TEXTURE_WRAP_S, sampler.wrap ) );
	GL_CHECK( glTexParameteri( target, GL_TEXTURE_WRAP_T, sampler.wrap ) );
	GL_CHECK( glTexParameteri( target, GL_TEXTURE_MAG_FILTER,
		sampler.magFilter ) );
	GL_CHECK( glTexParameteri( target, GL_TEXTURE_MIN_FILTER,
		sampler.minFilter) );

	GL_CHECK( glTexImage2D( GL_TEXTURE_2D, 0, sampler.internalFormat,
			grid.xEnd - grid.xStart,
			grid.yEnd - grid.yStart,
			0,
			sampler.format,
			GL_UNSIGNED_BYTE, data ) );

	GL_CHECK( glBindTexture( target, 0 ) );
}

std::vector< glm::ivec2 > GenTextureData(
	gTexture_t* tt,
	const gImageParams_t& canvasParams )
{
	std::vector< glm::ivec2 > dimensions;

	if ( !tt )
	{
		return dimensions;
	}

	const gTexConfig_t& sampler = GetTexConfig( canvasParams.sampler );

	uint16_t xDivide = 0, yDivide = 0;
	{
		gImageParams_t subdivision;
		subdivision.width = canvasParams.width;
		subdivision.height = canvasParams.height;
		subdivision.data = canvasParams.data;

		GLint gpuMaxDims;
		GL_CHECK( glGetIntegerv( GL_MAX_TEXTURE_SIZE, &gpuMaxDims ) );

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

		MLOG_INFO( "GRID COUNT: %i", tt->numGrids );

		dimensions.resize( tt->numGrids,
			glm::ivec2( subdivision.width,
				subdivision.height ) );

		tt->grids[ 0 ].xStart = tt->grids[ 0 ].yStart = 0;
		tt->grids[ 0 ].xEnd = dimensions[ 0 ].x;
		tt->grids[ 0 ].yEnd = dimensions[ 0 ].y;

		// NOTE: as it stands, each dimension value
		// in the vector is the same.

		// The following loop uses the dimension values
		// for the origin computations in such a manner that,
		// if more than 1 grid _doess_ exist, then their origins will
		// increase diagonally, instead of horizontally.

		// This may not matter from a performance persective, though,
		// because each grid represents its own texture, and anything
		// sampling for a given image is done completely relative
		// to the grid it belongs to.

		// It's clear that the current implementation wasn't finished
		// for handling multiple grids properly,
		// given that only the first grid is actually allocated
		// after the loop.
		for ( uint16_t i = 1; i < tt->numGrids; ++i )
		{
			tt->grids[ i ].xStart = tt->grids[ i - 1 ].xEnd;
			tt->grids[ i ].yStart = tt->grids[ i - 1 ].yEnd;
			tt->grids[ i ].xEnd = tt->grids[ i ].xStart + dimensions[ i ].x;
			tt->grids[ i ].yEnd = tt->grids[ i ].yStart + dimensions[ i ].y;
		}
	}

	// Provide the number of horizontal/vertical subdivisions, for
	// grid iteration
	dimensions.push_back( glm::ivec2( xDivide, yDivide ) );

	GenGridTexture( tt->target, tt->grids[ 0 ], sampler,
		&canvasParams.data[ 0 ] );

	return dimensions;
}

void TryAllocDummy( void )
{
	if ( !gDummy && !gSamplers.empty() )
	{
		gImageParams_t params;
		params.sampler.id = 0;

		GSetImageBuffer( params, 16, 16, 0 );

		gDummy.reset( new gTexture_t() );
		gDummy->target = GL_TEXTURE_2D;
		gDummy->sampler.id = 0;

		GenTextureData( gDummy.get(), params );

		gTextureImage_t data;

		gDummy->dimensions.push_back( glm::ivec2(
			params.width, params.height
		) );

		data.dimension = 0;
		data.gridOffset = glm::vec2( 0.0f );

		gDummy->imageSlots.push_back( data );
		gDummy->megaDims = glm::ivec2( params.width, params.height );
		gDummy->invRowPitch = glm::vec2( 1.0f, 1.0f );
	}
}

gGrid_t* GridFromSlot( const gTexture_t* t, const gTextureImage_t& image )
{
	if ( t->numGrids == 1 )
	{
		return t->grids;
	}

	// FIXME: what is the relationship between atlasPositionMap_t::origin,
	// data.gridLocation, and how they're used to:
	// a) compute the proper memory region offset for a given image
	// corresponding to a particular grid, and
	// b) used to find the grid which owns a given image, as shown in the
	// loop below?
	// Furthermore, do you know for certain that all of this fits properly
	// together, despite the fact that the grid x and y start/end positions
	// increase diagonally, as opoposed to existing within the canvas itself?

	// For the latter: yes, the relationship between the location of the
	// grid's actual position/dimensions and the data's grid location
	// is also flawed

	const glm::ivec2& dims = t->dimensions[ image.dimension ];

	for ( uint8_t i = 0; i < t->numGrids; ++i )
	{
		if ( image.gridOffset.x < t->grids[ i ].xStart ) continue;
		if ( image.gridOffset.x + ( float ) dims.x >= t->grids[ i ].xEnd ) continue;

		if ( image.gridOffset.y < t->grids[ i ].yStart ) continue;
		if ( image.gridOffset.y +  ( float ) dims.y >= t->grids[ i ].yEnd ) continue;

		return &t->grids[ i ];
	}

	return nullptr;
}

gGrid_t* GridFromSlot( gTextureHandle_t handle, uint32_t slotIndex )
{
	if ( slotIndex == G_UNSPECIFIED )
	{
		return nullptr;
	}

	const gTextureImage_t& image = GTextureImage( handle, slotIndex );
	const gTexture_t* t = GetTexture( handle );

	return GridFromSlot( t, image );
}

atlasBaseInfo_t CalcGridDimensions(
	gImageParams_t& canvasParams, gSamplerHandle_t sampler,
	gImageParamList_t& images )
{
	GLint maxTextureSize;
	GL_CHECK( glGetIntegerv( GL_MAX_TEXTURE_SIZE, &maxTextureSize ) );

	atlasBaseInfo_t baseInfo = AtlasGenOrigins( images,
		( uint16_t )maxTextureSize );
/*
	glm::vec2 maxDims( std::numeric_limits< float >::min() ),
			minDims( std::numeric_limits< float >::max() );

	for ( const glm::vec2& origin: baseInfo.origins )
	{
		if ( origin.x > maxDims.x ) maxDims.x = origin.x;
		if ( origin.y > maxDims.y ) maxDims.y = origin.y;
		if ( origin.x < minDims.x ) minDims.x = origin.x;
		if ( origin.y < minDims.y ) minDims.y = origin.y;
	}

	int32_t w = ( int32_t )( maxDims.x - minDims.x );
	int32_t h = ( int32_t )( maxDims.y - minDims.y );

	canvasParams.width = NextPower2( w );
	canvasParams.height = NextPower2( h );
	canvasParams.sampler = sampler;
	*/

	canvasParams.width = baseInfo.width;
	canvasParams.height = baseInfo.height;
	canvasParams.sampler = sampler;

	MLOG_INFO( "canvas width: %i, canvas height: %i",
 		canvasParams.width, canvasParams.height );

	GValidTextureDimensions( canvasParams.width, canvasParams.height );

	return baseInfo;
}

void GenSubdivision(
	gTexture_t* tt,
	uint16_t gridX,
	uint16_t gridY,
	uint16_t stride,
	const glm::ivec2& slotDims,
	const gTextureMakeParams_t& makeParams,
	const gImageParams_t& canvasParams,
	const atlasBaseInfo_t& baseInfo )
{
	UNUSED( canvasParams );

	const gTexConfig_t& sampler = GetTexConfig( makeParams.sampler );

	uint8_t grid = gridY * stride + gridX;

	GL_CHECK( glBindTexture( tt->target, tt->grids[ grid ].handle ) );

	uint16_t x = 0, y = 0;
	uint16_t square = NextSquare( baseInfo.origins.size() );

	glm::vec2 invPitchStride( 1.0f / ( float ) slotDims.x,
							  1.0f / ( float ) slotDims.y );

	tt->grids[ grid ].invStride = invPitchStride.x;
	tt->grids[ grid ].invPitch = invPitchStride.y;

	for ( size_t i = 0; i < baseInfo.origins.size(); ++i )
	{
		if ( !InRange( tt->grids[ grid ], baseInfo.origins[ i ],
			makeParams.images[ i ] ) )
		{
			continue;
		}

		// NOTE:
		// gridX/gridY refer to the (x, y) index of a particular grid
		// within the subtexture grid array. Each grid has its own
		// texture handle, and slotDims represents the size of the actual grid.

		// in the glTexSubImage2D call below, the idea for the origin x, y
		// params in the texture is to compute atlasPos.origin _relative_
		// to the grid, as opposed to it being computed relative to the
		// entire canvas as it originally was within the atlas generator.

		// The only reason why the following computations work, though,
		// is because slotDims is a uniform size for every grid. If it were
		// possible that each grid had a different size, this wouldn't work
		// properly.
		GL_CHECK( glTexSubImage2D(
			tt->target,
			0,
			( GLint )( baseInfo.origins[ i ].x - gridX * slotDims.x ),
			( GLint )( baseInfo.origins[ i ].y - gridY * slotDims.y ),
			makeParams.images[ i ].width,
			makeParams.images[ i ].height,
			sampler.format,
			GL_UNSIGNED_BYTE,
			&makeParams.images[ i ].data[ 0 ] ) );

		gTextureImage_t data;
		data.gridOffset = baseInfo.origins[ i ];

		{
			glm::ivec2 imageDims( makeParams.images[ i ].width,
				makeParams.images[ i ].height );

			auto dimIter = std::find( tt->dimensions.begin(),
				tt->dimensions.end(), imageDims );

			if ( dimIter != tt->dimensions.end() )
			{
				data.dimension = ( uint16_t )
					( dimIter - tt->dimensions.begin() );
			}
			else
			{
				data.dimension = tt->dimensions.size();
				tt->dimensions.push_back( imageDims );
			}
		}

		//data.stOffsetStart = baseInfo.origins[ i ] * invPitchStride;

		if ( tt->keyMapped )
		{
			uint16_t slot = makeParams.keyMaps[
				( gTextureImageKey_t )( y * square + x ) ];
			tt->keyMapSlots[ slot ] = data;
		}
		else
		{
			tt->imageSlots[ y * square + x ] = data;
		}

		uint16_t next = ( x + 1 ) % square;

		if ( next == 0 )
		{
			y++;
		}

		x = next;
	}

	GL_CHECK( glBindTexture( tt->target, 0 ) );
}

gTexture_t* MakeTexture( gTextureMakeParams_t& makeParams )
{
	gImageParams_t canvasParams;

	atlasBaseInfo_t baseInfo =
			CalcGridDimensions( canvasParams, makeParams.sampler,
				makeParams.images );

	gTexture_t* tt = new gTexture_t();

	tt->target = GL_TEXTURE_2D;
	tt->keyMapped = !!( makeParams.flags & G_TEXTURE_STORAGE_KEY_MAPPED_BIT );
	tt->megaDims = glm::ivec2( canvasParams.width, canvasParams.height );

	if ( !tt->keyMapped )
	{
		tt->imageSlots.resize( canvasParams.width * canvasParams.height );
	}

	std::vector< glm::ivec2 > dims = GenTextureData( tt, canvasParams );

	const glm::ivec2& splitInfo = dims[ dims.size() - 1 ];

	for ( uint16_t y = 0; y < ( 1 << splitInfo.y ); ++y )
	{
		for ( uint16_t x = 0; x < ( 1 << splitInfo.x ); ++x )
		{
			GenSubdivision(
				tt,
				x, y,
				( 1 << splitInfo.x ),
				dims[ y * ( 1 << splitInfo.x ) + x ],
				makeParams,
				canvasParams,
				baseInfo );
		}
	}

	return tt;
}

bool DetermineImageFormats( int8_t bpp, uint32_t& format,
	uint32_t& internalFormat )
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

void GStageSlot( gTexSlot_t slot )
{
	gSlotStage = slot;
}

void GUnstageSlot( void )
{
	gSlotStage = ( gTexSlot_t ) G_UNSPECIFIED;
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

void GBindGrid( const gTextureHandle_t& handle, uint32_t grid, uint32_t offset )
{
	const gTexture_t* t = GetTexture( handle );

	if ( t && grid < t->numGrids )
	{
		GL_CHECK( glActiveTexture( GL_TEXTURE0 + offset ) );
		GL_CHECK( glBindTexture( t->target, t->grids[ grid ].handle ) );
	}
}

const gTextureImage_t& GTextureImage( const gTextureHandle_t& handle,
	uint32_t slot )
{
	const gTexture_t* t = GetTexture( handle );

	// Ensure we don't have something like a negative texture index
	if ( t == gDummy.get() || ( int32_t )slot < 0 )
	{
		slot = 0;
	}

	return t->GetSlot( slot );
}

const glm::ivec2& GTextureImageDimensions( const gTextureHandle_t& handle,
	const gTextureImage_t& image )
{
	const gTexture_t* t = GetTexture( handle );

	if ( image.dimension >= t->dimensions.size() )
	{
		MLOG_ERROR( "Found invalid image @ ( x, y ) = ( %f, %f )" \
			" for handle %lu",
			image.gridOffset.x, image.gridOffset.y, handle.id );
	}

	return t->dimensions[ image.dimension ];
}

gTextureImageShaderParams_t GTextureImageShaderParams(
	const gTextureHandle_t& handle,
 	uint32_t slot )
{
	const gTexture_t* tt = GetTexture( handle );
	const gTextureImage_t& image = GTextureImage( handle, slot );
	gGrid_t* grid = GridFromSlot( tt, image );

	gTextureImageShaderParams_t params;

	params.transform.x = image.gridOffset.x * grid->invStride;
	params.transform.y = image.gridOffset.y * grid->invPitch;
	params.transform.z = grid->invStride;
	params.transform.w = grid->invPitch;

	params.dimensions.x = ( float ) tt->dimensions[ image.dimension ].x;
	params.dimensions.y = ( float ) tt->dimensions[ image.dimension ].y;

	return params;
}

const gTextureImage_t& GTextureImage( const gTextureHandle_t& handle )
{
	MLOG_ASSERT( !G_VNULL( gSlotStage ), "Not slot stage set. Handle: %i",
		handle.id );

	return GTextureImage( handle, gSlotStage );
}

uint16_t GTextureImageCount( const gTextureHandle_t& handle )
{
	const gTexture_t* tex = GetTexture( handle );

	if ( tex )
	{
		if ( tex->keyMapped )
		{
			return tex->keyMapSlots.size();
		}
		else
		{
			return tex->imageSlots.size();
		}
	}

	return ( uint16_t ) G_UNSPECIFIED;
}

gTextureImageKeyList_t GTextureImageKeys( const gTextureHandle_t& handle )
{
	const gTexture_t* tex = GetTexture( handle );

	gTextureImageKeyList_t keyList;

	if ( tex->keyMapped )
	{
		keyList.reserve( tex->keyMapSlots.size() );

		for ( const auto& kv: tex->keyMapSlots )
		{
			keyList.push_back( kv.first );
		}
	}
	else
	{
		keyList.reserve( tex->imageSlots.size() );

		for ( uint32_t i = 0; i < tex->imageSlots.size(); ++i )
		{
			keyList.push_back( i );
		}
	}

	return keyList;
}

uint16_t GTextureGridCount( const gTextureHandle_t& handle )
{
	const gTexture_t* tex = GetTexture( handle );

	if ( tex )
	{
		return tex->numGrids;
	}

	return ( uint16_t ) G_UNSPECIFIED;
}


glm::vec2 GTextureInverseRowPitch( const gTextureHandle_t& handle )
{
	MLOG_ASSERT( !G_VNULL( gSlotStage ), "No slot stage lroaded for handle: %i", handle.id );

	gGrid_t* grid = GridFromSlot( handle, gSlotStage );

	return glm::vec2( grid->invStride, grid->invPitch );
}

uint16_t GTextureMegaWidth( const gTextureHandle_t& handle )
{
	return GetTexture( handle )->megaDims.x;
}

uint16_t GTextureMegaHeight( const gTextureHandle_t& handle )
{
	return GetTexture( handle )->megaDims.y;
}

bool GValidTextureDimensions( uint16_t width, uint16_t height )
{
	GLint maxTextureSize;
	GL_CHECK( glGetIntegerv( GL_MAX_TEXTURE_SIZE, &maxTextureSize ) );

	if ( width > maxTextureSize || height > maxTextureSize )
	{
		MLOG_INFO( "Texture Size is Invalid;"\
			" (max value, desired width, desired height) => "\
			" (%iu, %iu, %iu)", maxTextureSize, width, height );
		return false;
	}

	return true;
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
							const uint8_t* sourceData,
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

bool GLoadImageFromMemory( gImageParams_t& image,
	const std::vector< uint8_t >& buffer,
 	int32_t width, int32_t height, int32_t bpp )
{
	if ( image.sampler.id == G_UNSPECIFIED )
	{
		MLOG_WARNING( "image passed that\'s missing a sampler" );
		return false;
	}

	if ( bpp != gSamplers[ image.sampler.id ].bpp )
	{
		uint32_t numPixels = width * height;
		image.data.resize( numPixels * gSamplers[ image.sampler.id ].bpp, 255 );
		GSetAlignedImageData( image, &buffer[ 0 ], bpp, numPixels );
	}
	else
	{
		image.data = std::move( buffer );
	}

	image.width = width;
	image.height = height;

	return true;
}

bool GLoadImageFromFile( const std::string& imagePath, gImageParams_t& image )
{
	int32_t width, height, bpp;

	std::vector< uint8_t > tmp;
	if ( !File_GetPixels( imagePath, tmp, bpp, width, height ) )
	{
		return false;
	}

	return GLoadImageFromMemory( image, tmp, width, height, bpp );
}
