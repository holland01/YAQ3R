#include "testiowebworker.h"
#ifdef EMSCRIPTEN

#include "em_api.h"
#include "io.h"
#include "q3bsp.h"
#include "renderer/texture.h"
#include "effect_shader.h"

static void OnFrameIteration( void )
{
	static volatile int k = 0;
	k++;
	if ( k % 1000 == 0 )
	{
		MLOG_INFO( "Beep" );
	}
}

static void OnReadFinish( void )
{
	MLOG_INFO( "DONE!" );
}

static void ReadCallback( char* data, int size, void* param )
{
	EM_FWW_Copy( data, size, param );

	const std::vector< unsigned char >& v =
		*( ( std::vector< unsigned char >* )param );

	MLOG_INFO( "Job's finished. Size: %i bytes", v.size() );
}

IOTestWebWorker::IOTestWebWorker( void )
{
	InitSysLog();
	EM_MountFS();
}

IOTestWebWorker::~IOTestWebWorker( void )
{
	KillSysLog();
	EM_UnmountFS();
}

int IOTestWebWorker::operator()( void )
{
	std::vector< gImageParams_t > textures;
	gSamplerHandle_t imageSampler;

	std::vector< unsigned char > buffer;

	Q3BspMap map;
	map.Read( ASSET_Q3_ROOT"/maps/q3dm2.bsp", 1, OnReadFinish );

	emscripten_set_main_loop( OnFrameIteration, 0, 1 );

	return 0;
}
#endif
