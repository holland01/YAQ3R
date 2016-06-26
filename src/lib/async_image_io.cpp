#include "async_image_io.h"
#include "q3bsp.h"
#include "io.h"
#include "worker/wapi.h"

void gImageLoadTracker_t::LogImages( void )
{
	std::stringstream ss;
	ss << "Reading Images: \n";
	for ( size_t i = 0; i < gImageTracker->textureInfo.size(); ++i )
	{
		ss << gImageTracker->textureInfo[ i ].path;

		if ( i < gImageTracker->textureInfo.size() - 1 )
		{
			ss << ", ";
		}

		if ( ( ( i + 1 ) & 0x3 ) == 0 )
		{
			ss << "\n";
		}
	}

	printf( "---------\n%s\n----------\n", ss.str().c_str() );
};

std::unique_ptr< gImageLoadTracker_t > gImageTracker( nullptr );


#define DATA_FMT_STRING( bufferSize ) \
	"Received: width->%i, height->%i, bpp->%i, size->%i, copy buffer size->%i,"\
 	"For path: \'%s\'.",\
	width, height, bpp, size, ( bufferSize ),\
	gImageTracker->textureInfo[ gImageTracker->iterator ].path.c_str()

// It's assumed that lightmaps aren't passed into this callback...

// First 8 bytes follow this format:
// 0, 1 -> width
// 2, 3 -> height
// 4 -> bpp
// 5, 6, 7 -> padding
// 8, width * height * bpp -> image data

static void OnImageRead( char* buffer, int size, void* param )
{
	if ( !gImageTracker )
	{
		MLOG_ERROR( "Image tracker is NULL" );
		return;
	}

	UNUSED(param);	

	// We may have an invalid path, or a path which exists but with
	// a different extension
 	
	uint32_t testDims = WAPI_Fetch32( buffer, size, 0 );
	
	printf( "first 4: { %i, %i, %i, %i }; Size: %i\n", buffer[ 0 ],
		buffer[ 1 ], buffer[ 2 ], buffer[ 3 ], size );
	printf( "Value from WAPI: %x\n", testDims );

	if ( !testDims )
	{
		goto next_image;
	}

	// Grab image data that we need; copy it over.
	{
		int32_t width = WAPI_Fetch16( buffer, 0, size );
		int32_t height = WAPI_Fetch16( buffer, 2, size );
		int32_t bpp = ( int32_t ) buffer[ 4 ];

		if ( !width || !height || !bpp )
		{
			MLOG_ERROR( "zero portion of metadata received. " 
				DATA_FMT_STRING( 0 ) );
			return;
		}

		std::vector< uint8_t > imageData( width * height * bpp, 0 );

		MLOG_ASSERT( ( unsigned ) size == imageData.size() + 8,
			"buffer size does not match "\
	 		"interpreted metadata criteria. " DATA_FMT_STRING( 
	 			imageData.size() ) );

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
	if ( ( unsigned ) ++gImageTracker->iterator == 
			gImageTracker->textureInfo.size() )
	{
		if ( gImageTracker->finishEvent )
		{
			gImageTracker->finishEvent( gImageTracker.get() );
		}
	}
	else
	{
		const std::string& next =
			gImageTracker->textureInfo[ gImageTracker->iterator ].path;

		gFileWebWorker.Await( OnImageRead, "ReadImage", next, nullptr );
	}
}
#undef DATA_FMT_STRING

gPathMap_t AIIO_MakeAssetPath( const char* path )
{
	gPathMap_t pm;

	if ( !path )
	{
		return pm;
	}

	{
		std::stringstream ss;
		ss << ASSET_Q3_ROOT;
		if ( path[ 0 ] != '/' )
		{
			ss << '/';
		}
		ss << path;
		pm.path = ss.str();
	}

	return pm;
}

void AIIO_ReadImages( Q3BspMap& map, std::vector< gPathMap_t > pathInfo, 
	gSamplerHandle_t sampler, onFinishEvent_t finish, 
	onFinishEvent_t insert )
{
	gImageTracker.reset( new gImageLoadTracker_t( map, pathInfo ) );
	gImageTracker->sampler = sampler;
	gImageTracker->finishEvent = finish;
	gImageTracker->insertEvent = insert;

	gImageTracker->LogImages();

	gFileWebWorker.Await( OnImageRead, "ReadImage",
		gImageTracker->textureInfo[ 0 ].path, nullptr );
}
