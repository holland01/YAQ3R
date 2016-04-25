#include "async_image_io.h"
#include "q3bsp.h"
#include "io.h"
#include "worker/wapi.h"

bool gImageLoadTracker_t::NextFallback( void )
{
	if ( FallbackEnd() )
	{
		return false;
	}

	std::string ext;
	size_t index = 0;
	if ( File_GetExt( ext, &index, textureInfo[ iterator ].path ) )
	{
		MLOG_INFO( "Before: \'%s\'", textureInfo[ iterator ].path.c_str() );

		// Same extension, which means we're done if this is the last
		// This is possible in the event that, say, a jpeg is passed in initially,
		// not found, and then the next path falls back to a jpeg (because most image
		// paths represent tga files, so using tga as a fallback is...less important).

		// FIXME: The above parens, though, brings up a significant point: if we have
		// a path which is initially a .jpeg, but there is no fallback mechanism which
		// considers the possibility that the initial jpeg may actually be a tga file
		// (instead of the other way around), then we're potentially missing an image
		// which could otherwise be loaded if we take this into account.

		// Since the current scheme accepts only a static string of text files,
		// we can do something along the lines of iterate through the entire
		// set initially, and remove the item which is equivalent to the extension
		// of the initial path.
		if ( ext == fallbackExts[ extIterator ] )
		{
			if ( ( extIterator + 1 ) == ( int16_t )fallbackExts.size() )
			{
				return false;
			}
		}

		textureInfo[ iterator ].path =
			textureInfo[ iterator ].path.substr( 0, index + 1 )
				+ fallbackExts[ extIterator ];

		MLOG_INFO( "(ext: %s) After: \'%s\'",
			fallbackExts[ extIterator ].c_str(),
			textureInfo[ iterator ].path.c_str() );

		extIterator++;

		return true;
	}
	else
	{
		MLOG_ERROR( "File \'%s\' doesn't have an extension - this is weird.",
	 	 textureInfo[ iterator ].path.c_str() );

		 return false;
	}
}

std::unique_ptr< gImageLoadTracker_t > gImageTracker( nullptr );


#define DATA_FMT_STRING( bufferSize ) \
	"Received: width->%i, height->%i, bpp->%i, size->%i, copy buffer size->%i,"\
 	"For path: \'%s\'.",\
	width, height, bpp, size, ( bufferSize ),\
	gImageTracker->textureInfo[ gImageTracker->iterator ].path.c_str()

// It's assumed that lightmaps are _not_ passed into this callback...

// First 8 bytes follow this format:
// 0, 1 -> width
// 2, 3 -> height
// 4 -> bpp
// 5, 6, 7 -> padding
// 8, width * height * bpp -> image data
static void OnImageRead( char* buffer, int size, void* param )
{
	MLOG_ASSERT( !!gImageTracker, "gImageTracker is null" );

	// We may have an invalid path, or a path which exists but with
	// a different extension
 	if ( !WAPI_FetchInt( buffer, size ) )
	{
		// If NextFallback is good, we can re-check the same image with its
		// new extension
		if ( gImageTracker->NextFallback() )
		{
			goto query_image;
		}

		// Screw it: let's just move on.
		gImageTracker->ResetFallback();
		goto next_image;
	}

	// Grab image data that we need; copy it over.
	{
		int32_t width = ( int32_t ) buffer[ 0 ] | ( ( int32_t ) buffer[ 1 ] << 8 );
		int32_t height = ( int32_t ) buffer[ 2 ] | ( ( int32_t ) buffer[ 3 ] << 8 );
		int32_t bpp = ( int32_t ) buffer[ 4 ];

		if ( !width || !height || !bpp )
		{
			MLOG_ERROR( "zero portion of metadata received. " DATA_FMT_STRING( 0 ) );
			return;
		}

		std::vector< uint8_t > imageData( width * height * bpp, 0 );

		MLOG_ASSERT( ( unsigned ) size == imageData.size() + 8,
			"buffer size does not match "\
	 		"interpreted metadata criteria. " DATA_FMT_STRING( imageData.size() ) );

		memcpy( &imageData[ 0 ], buffer + 8, imageData.size() );

		// Ensure it conforms to our standards
		gImageParams_t image;
		image.sampler = gImageTracker->sampler;

		if ( !GLoadImageFromMemory( image, imageData, width, height, bpp ) )
		{
			MLOG_ERROR( "Failure to load image data. "\
				DATA_FMT_STRING( imageData.size() ) );
			return;
		}
		MLOG_INFO( DATA_FMT_STRING( imageData.size() ) );

		if ( gImageTracker->insertEvent )
		{
			gImageTracker->insertEvent( gImageTracker.get() );
		}

		gImageTracker->textures.push_back( image );
	}

next_image:
	// Are we finished?
	if ( ( unsigned ) ++gImageTracker->iterator == gImageTracker->textureInfo.size() )
	{
		if ( gImageTracker->finishEvent )
		{
			gImageTracker->finishEvent( gImageTracker.get() );
		}

		gImageTracker.release();
		return;
	}

	// No; query the next path in the list
query_image:
	{
		const std::string& next =
			gImageTracker->textureInfo[ gImageTracker->iterator ].path;

		gFileWebWorker.Await( OnImageRead, "ReadImage", next, nullptr );
	}
}
#undef DATA_FMT_STRING

void AIIO_ReadImages( Q3BspMap& map, std::vector< gPathMap_t > pathInfo,
	std::vector< std::string > fallbackExts, gSamplerHandle_t sampler,
	onFinishEvent_t finish, onFinishEvent_t insert )
{
	gImageTracker.reset( new gImageLoadTracker_t( map, std::move( pathInfo ),
 		std::move( fallbackExts ) ) );
	gImageTracker->sampler = sampler;
	gImageTracker->finishEvent = finish;
	gImageTracker->insertEvent = insert;

	gFileWebWorker.Await( OnImageRead, "ReadImage",
		gImageTracker->textureInfo[ 0 ].path, nullptr );
}
