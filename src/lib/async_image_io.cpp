#include "async_image_io.h"
#include "q3bsp.h"

std::unique_ptr< gImageLoadTracker_t > gImageTracker;

// It's assumed that lightmaps are _not_ passed into this callback...

#define DATA_FMT_STRING\
	"Received: width->%i, height->%i, bpp->%i, size->%i, copy buffer size->%i,"\
 	"For path: \'%s\'.",\
	width, height, bpp, size, imageData.size(),\
	gImageTracker->textureInfo[ gImageTracker->iterator ].path

// First 8 bytes follow this format:
// 0, 1 -> width
// 2, 3 -> height
// 4 -> bpp
// 5, 6, 7 -> padding
// 8, width * height * bpp -> image data
static void OnImageRead( char* buffer, int size, void* param )
{
	MLOG_ASSERT( !!gImageTracker, "gImageTracker is null" );

	// null buffer may just indicate an invalid path, so we forget about it.
	if ( !buffer )
	{
		goto next_image;
	}

	// Grab image data that we need; copy it over.
	{
		int32_t width = ( int32_t ) buffer[ 0 ] | ( ( int32_t ) buffer[ 1 ] << 8 );
		int32_t height = ( int32_t ) buffer[ 2 ] | ( ( int32_t ) buffer[ 3 ] << 8 );
		int32_t bpp = ( int32_t ) buffer[ 4 ];

		if ( !width || !height || !bpp )
		{
			MLOG_ERROR( "zero portion of metadata received"DATA_FMT_STRING );
			return;
		}

		std::vector< uint8_t > imageData( width * height * bpp, 0 );

		MLOG_ASSERT( size == imageData.size() + 8, "buffer size does not match "\
	 		"interpreted metadata criteria."DATA_FMT_STRING );

		memcpy( &imageData[ 0 ], buffer + 8, imageData.size() );

		// Ensure it conforms to our standards
		gImageParams_t image;
		image.sampler = gImageTracker->sampler;

		if ( !GLoadImageFromMemory( image, imageData, width, height, bpp ) )
		{
			MLOG_ERROR( "Failure to load image data."DATA_FMT_STRING );
			return;
		}

		if ( gImageTracker->insertEvent )
		{
			gImageTracker->insertEvent( gImageTracker.get() );
		}

		gImageTracker->textures.push_back( image );
	}

next_image:
	// Are we finished?
	if ( ++gImageTracker->iterator == gImageTracker->textureInfo.size() )
	{
		if ( gImageTracker->finishEvent )
		{
			gImageTracker->finishEvent( gImageTracker.get() );
		}

		gImageTracker.release();
	}
	// No; query the next path in the list
	else
	{
		const std::string& next =
			gImageTracker->textureInfo[ gImageTracker->iterator ].path;

		gFileWebWorker.Await( OnImageRead, "ReadImageFile", next, nullptr );
	}
}
#undef DATA_FMT_STRING

void AIIO_ReadImages( Q3BspMap& map, std::vector< gPathMap_t > pathInfo,
	gImageSampler_t sampler, onFinishEvent_t finish, onFinishEvent_t insert )
{
	gImageTracker.reset( new gImageLoadTracker_t( map, std::move( pathInfo ) ) );
	gImageTracker->sampler = sampler;
	gImageTracker->finishEvent = finish;
	gImageTracker->insertEvent = insert;

	gFileWebWorker.Await( OnImageRead, "ReadImageFile",
		gImageTracker->textureInfo[ 0 ].path, nullptr );
}
