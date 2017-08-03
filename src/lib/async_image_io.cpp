#include "async_image_io.h"
#include "q3bsp.h"
#include "io.h"
#include "worker/wapi.h"
#include "em_api.h"
#include "extern/gl_atlas.h"

static gImageLoadTrackerPtr_t gImageTracker( nullptr );

void gImageLoadTracker_t::LogImages( void )
{
	std::stringstream ss;
	ss << "Reading Images: \n";

	uint32_t i = 0;

	for ( auto& kv : gImageTracker->textureInfo )
	{
		ss << kv.first;

		if ( i < gImageTracker->textureInfo.size() - 1 )
		{
			ss << ", ";
		}

		if ( ( ( i + 1 ) & 0x3 ) == 0 )
		{
			ss << "\n";
		}

		++i;
	}

	printf( "---------\n%s\n----------\n", ss.str().c_str() );
};

#define DATA_FMT_STRING( bufferSize ) \
	"Received: width->%i, height->%i, bpp->%i, size->%i, copy buffer size->%i,"\
 	"For path: \'%s\'.",\
	imageInfo->width, imageInfo->height, imageInfo->bpp, size, ( bufferSize ),\
	&imageInfo->name[ 0 ]

// It's assumed that lightmaps aren't passed into this callback...
static void AssignIndex( wApiImageInfo_t* info, uint16_t assignIndex )
{
	std::string pathString( info->name );

	if ( gImageTracker->isKeyMapped )
	{
		size_t keyMap =
			( size_t ) gImageTracker->textureInfo[ pathString ];

		gImageTracker->destAtlas->map_key_to_image(
			keyMap,
			assignIndex
		);

	//	if ( assignIndex == gla::atlas_t::no_image_index )
	//	{
	//		gImageTracker->map->MarkBadTexture( keyMap );
	//	}
	}
	else
	{
		shaderStage_t* stage =
			( shaderStage_t* ) gImageTracker->textureInfo[ pathString ];

		// This index will persist in the texture array it's going into
		stage->textureIndex = assignIndex;
	}

	gImageTracker->iterator++;
}

// Bytes follow this format:

// [0, 63] -> filepath
// 64, 65 -> width
// 66, 67 -> height
// 68 -> bpp
// [69, 71] -> padding

// What follows afterward is the image data.

// The first send will always report the amount of file paths actually
// accepted: sometimes a client-requested path or two won't actually be on
// disk, and nothing similar will be available either; if we don't actually
// send this then there's a good chance that that the finish function
// won't be called.

// Once finished, we send the address of gImageTracker to
// gImageTracker->finishEvent(). gImageTracker is a unique_ptr.
static void OnImageRead( char* buffer, int size, void* param )
{
	if ( !gImageTracker )
	{
		MLOG_ERROR( "%s", "gImageTracker is NULL" );
		return;
	}

	if ( !gImageTracker->finishEvent )
	{
		MLOG_ERROR( "%s", "No finishEvent assigned in gImageTracker!" );
		return;
	}

	wApiImageInfo_t* imageInfo = ( wApiImageInfo_t* ) buffer;

	bool atEnd = gImageTracker->serverImageCount > 0
		&& gImageTracker->iterator >= gImageTracker->serverImageCount;

	if ( atEnd )
	{
		gImageTracker->finishEvent( &gImageTracker );

		return;
	}

	if ( !imageInfo )
	{
		MLOG_ERROR( "%s", "nullptr should NOT be received at this stage" );

		return;
	}

	// A zero size implies an image which couldn't be found, so we
	// take the name from our info and use it to assign an invalid index.
	if ( !size )
	{
		AssignIndex(
			imageInfo,
			gla::atlas_t::no_image_index
		);

		return;
	}

	if ( !gImageTracker->serverImageCount )
	{
		if ( strncmp(
				&imageInfo->name[ 0 ],
				WAPI_IMAGE_SERVER_IMAGE_COUNT,
				strlen( WAPI_IMAGE_SERVER_IMAGE_COUNT ) ) == 0 )
		{
			gImageTracker->serverImageCount = ( size_t ) imageInfo->width;

			// There may not be any needed images for this current bundle.
			// Even if that's the case, the server will still provide its
			// final "ending" call, so here we make sure that atEnd will
			// resolve to true naturally on the final run.
			if ( !gImageTracker->serverImageCount )
			{
				gImageTracker->serverImageCount = 0x7FFFFFFF;
				gImageTracker->iterator = 0x7FFFFFFF;
			}
		}
		else
		{
			MLOG_ERROR(
				"%s",
				"ERROR: first wApiImageInfo_t sent MUST be the header"
			);
		}

		return;
	}

	if ( !imageInfo->width || !imageInfo->height || !imageInfo->bpp )
	{
		MLOG_ERROR(
			"zero portion of metadata received. " DATA_FMT_STRING( 0 ) );
		return;
	}

	size_t imageDataSize = imageInfo->width * imageInfo->height * imageInfo->bpp
		+ sizeof( *imageInfo );

	if ( ( size_t ) size != imageDataSize )
	{
		MLOG_ERROR(
			"buffer size does not match " \
	 		"interpreted metadata criteria. "
			DATA_FMT_STRING( imageDataSize )
		);
		return;
	}

	gla::push_atlas_image(
		*( gImageTracker->destAtlas ),
		( uint8_t* ) &buffer[ sizeof( *imageInfo ) ],
		imageInfo->width,
		imageInfo->height,
		imageInfo->bpp
	);

	AssignIndex(
	 	imageInfo,
		gImageTracker->destAtlas->num_images - 1
	);
}
#undef DATA_FMT_STRING

void AIIO_FixupAssetPath( gPathMap_t& pm )
{
	std::string rootFolder( "/" );
	rootFolder.append( ASSET_Q3_ROOT );

	if ( pm.path[ 0 ] != '/' )
	{
		rootFolder.append( 1, '/' );
	}

	pm.path = rootFolder + pm.path;
}

void AIIO_ReadImages(
	Q3BspMap* map,
	const std::string& bundlePath,
	const std::vector< gPathMap_t >& pathInfo,
	onFinishEvent_t finish,
	gla::atlas_t* destAtlas,
	bool keyMapped
)
{
	gImageTracker.reset(
		new gImageLoadTracker_t(
			map,
			pathInfo,
			finish,
			destAtlas,
			keyMapped
		)
	);

	std::stringstream bundlePaths;
	bundlePaths << bundlePath << ASSET_ASCII_DELIMITER;

	for ( uint32_t i = 0; i < pathInfo.size(); ++i )
	{
		bundlePaths << pathInfo[ i ].path;

		if ( ( i + 1 ) < pathInfo.size() )
		{
			bundlePaths << ASSET_ASCII_DELIMITER;
		}
	}

	gFileWebWorker.Await(
		OnImageRead,
		"MountPackage",
		bundlePaths.str(),
		nullptr
	);
}
