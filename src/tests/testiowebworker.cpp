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
	Q3BspMap map;
	std::vector< gImageParams_t > textures;
	gSamplerHandle_t imageSampler;

	map.Read( "asset/stockmaps/maps/q3dm2.bsp", 1 );
	S_LoadShaders( &map, imageSampler, textures );

	emscripten_set_main_loop( FrameIteration, 0, 1 );

	return 0;
}
#endif
