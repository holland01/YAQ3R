#include "testiowebworker.h"
#ifdef EMSCRIPTEN

#include "em_api.h"
#include "io.h"
#include "q3bsp.h"
#include "renderer/texture.h"
#include "effect_shader.h"

static void FrameIteration( void )
{
	static volatile int k = 0;
	k++;
	if ( k % 1000 == 0 )
	{
		MLOG_INFO( "Beep" );
	}
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

	File_GetBuf( buffer, "/asset/stockmaps/maps/q3dm2.bsp", ReadCallback );

	//map.Read( "asset/stockmaps/maps/q3dm2.bsp", 1 );
	//S_LoadShaders( &map, imageSampler, textures );

	emscripten_set_main_loop( FrameIteration, 0, 1 );

	return 0;
}
#endif
