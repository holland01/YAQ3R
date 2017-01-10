#include "async_image_io.h"
#include "q3bsp.h"
#include "io.h"
#include "worker/wapi.h"
#include "em_api.h"
#include "extern/gl_atlas.h"

static gImageLoadTracker_t* gImageTracker = nullptr;

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
// What follows is the image data.
static void OnImageRead( char* buffer, int size, void* param )
{
	if ( !gImageTracker )
	{
		MLOG_ERROR( "Image tracker is NULL" );
		return;
	}

	UNUSED( param );

	// We may have an invalid path, or a path which exists but with
	// a different extension

	uint32_t testDims = WAPI_Fetch32( buffer, size, 0 );

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

		size_t imageDataSize = width * height * bpp + 8;

		if ( ( unsigned ) size != imageDataSize )
		{
			MLOG_ERROR(
				"buffer size does not match "\
		 		"interpreted metadata criteria. "
				DATA_FMT_STRING( imageDataSize )
			);
			return;
		}

		gla::push_atlas_image(
			gImageTracker->destAtlas,
			( uint8_t* ) &buffer[ 8 ],
			width,
			height,
			bpp
		);

		if ( gImageTracker->isKeyMapped )
		{
			size_t keyMap =
				( size_t ) gImageTracker->
					textureInfo[ gImageTracker->iterator ].param;

			gImageTracker->destAtlas.map_key_to_image(
				keyMap,
				gImageTracker->destAtlas.num_images - 1
			);
		}
		else
		{
			shaderStage_t* stage =
				( shaderStage_t* )gImageTracker->
					textureInfo[ gImageTracker->iterator ].param;

			// This index will persist in the texture array it's going into
			stage->textureIndex = gImageTracker->destAtlas.num_images - 1;
		}
	}

next_image:
	if ( ( size_t ) ++gImageTracker->iterator ==
			gImageTracker->textureInfo.size() )
	{
		if ( gImageTracker->finishEvent )
		{
			gImageTracker->finishEvent( &gImageTracker );
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

void AIIO_FixupAssetPath( gPathMap_t& pm )
{
	std::string rootFolder( ASSET_Q3_ROOT );

	if ( pm.path[ 0 ] != '/' )
	{
		rootFolder += "/";
	}

	pm.path = rootFolder + pm.path;
}

void AIIO_ReadImages( Q3BspMap& map,
	std::vector< gPathMap_t > pathInfo,
	onFinishEvent_t finish,
	gla::atlas_t& destAtlas,
	bool keyMapped )
{
	gImageTracker = new gImageLoadTracker_t(
		map,
		pathInfo,
		finish,
		destAtlas,
		keyMapped
	);

	gFileWebWorker.Await( OnImageRead, "ReadImage",
		gImageTracker->textureInfo[ 0 ].path, nullptr );
}
