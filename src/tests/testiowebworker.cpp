#include "testiowebworker.h"
#ifdef EMSCRIPTEN

#include "em_api.h"
#include "io.h"
#include "q3bsp.h"
#include "renderer/texture.h"
#include "renderer/context_window.h"
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

static void OnReadFinish( void* param )
{
	UNUSED( param );
	MLOG_INFO( "DONE!" );
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
	//GInitContextWindow( "iotestwebworker", gContext );

	Q3BspMap map;
	map.Read( ASSET_Q3_ROOT"/maps/q3dm2.bsp", 1, OnReadFinish );

	emscripten_set_main_loop( OnFrameIteration, 0, 1 );
	return 0;
}
#endif
