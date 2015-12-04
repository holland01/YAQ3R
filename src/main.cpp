#include "common.h"
#include "renderer.h"
#include "io.h"
#include "tests/trenderer.h"
#include "tests/test_tessellation.h"
#include "tests/test_textures.h"

// Is global
void FlagExit( void )
{
    delete gAppTest;
    gAppTest = NULL;
	system( "pause" );
	exit( 0 );
}

#define SIZE_ERROR_MESSAGE "Unsupported type size found."

int main( int argc, char** argv )
{
	static_assert( sizeof( glm::vec3 ) == sizeof( float ) * 3, SIZE_ERROR_MESSAGE );
	static_assert( sizeof( glm::vec2 ) == sizeof( float ) * 2, SIZE_ERROR_MESSAGE );
	static_assert( sizeof( glm::ivec3 ) == sizeof( int ) * 3, SIZE_ERROR_MESSAGE );

    gAppTest = new TTextureTest();
    //gAppTest = new TessTest();
	gAppTest->Load();

    int code = gAppTest->Exec();

    FlagExit();

    return code;
	
	return 0;
}


