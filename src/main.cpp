#include "common.h"
#include "renderer.h"
#include "io.h"
#include "tests/trenderer.h"
#include "renderer/buffer.h"
#include <iostream>

#undef main

#ifdef EMSCRIPTEN
#	include <emscripten.h>
#endif

// Is global
void FlagExit( void )
{
	if ( gAppTest )
	{
		delete gAppTest;
		gAppTest = nullptr;
	}
#ifdef EMSCRIPTEN

	//emscripten_force_exit( 0 );
#else
	exit( 0 );
#endif
}

#define SIZE_ERROR_MESSAGE "Unsupported type size found."

int main( void )
{
	static_assert( sizeof( glm::vec3 ) == sizeof( float ) * 3, SIZE_ERROR_MESSAGE );
	static_assert( sizeof( glm::vec2 ) == sizeof( float ) * 2, SIZE_ERROR_MESSAGE );
	static_assert( sizeof( glm::ivec3 ) == sizeof( int ) * 3, SIZE_ERROR_MESSAGE );

	gAppTest = new TRenderer( ASSET_Q3_ROOT"/maps/Railgun_Arena.bsp" );
	gAppTest->Load();

	return 0;
}
