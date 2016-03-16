#include "test_atlas_struct.h"
#include "lib/random.h"
#include "lib/math.h"
#include "renderer/util.h"
#include "renderer/buffer.h"
#include "io.h"
#include <algorithm>
#include <unordered_map>

TAtlas::TAtlas( void )
	: Test( 800, 600, false ),
	  camera( new InputCamera() )
{
	context = SDL_GL_CONTEXT_PROFILE_COMPATIBILITY;
	camPtr = camera;
}

void TAtlas::Load( void )
{
	if ( !Test::Load( "woo" ) )
	{
		return;
	}

	camPtr->SetPerspective( 90.0f, 800.0f, 600.0f, 1.0f, 5000.0f );

	GEnableDepthBuffer();

	GL_CHECK( glClearColor( 0.0f, 0.0f, 0.0f, 1.0f ) );
	GL_CHECK( glPointSize( 10 ) );

	camPtr->moveStep = 0.1f;

	std::stringstream out;

	out << "Num Images: %i\n"
		<< "Max Image Dims: %i\n"
		<< "Total Res: %i x %i\n"
		<< "POT Total Res: %i x %i\n";

	out << "Widths: {\n";
	for ( uint16_t w: widths.store )
	{
		out << "\t" << std::to_string( w ) << "\n";
	}

	out << "}\nHeights: {\n";
	for ( uint16_t h: heights.store )
	{
		out << '\t' << std::to_string( h ) << '\n';
	}

	out << "}\n";

	MLOG_INFO( out.str().c_str(),
			   nImage, maxImage,
			   totalRes.x, totalRes.y,
			   NextPower2( totalRes.x ), NextPower2( totalRes.y ) );
#endif
}

void TAtlas::Run( void )
{
	const float Z_PLANE = -5.0f;

	camPtr->Update();

	std::for_each( boundsList.begin(), boundsList.end(), [ & ]( const bounds_t& box )
	{
		glm::mat4 v( camPtr->ViewData().transform );

		glm::vec3 s( box.origin, Z_PLANE );
		glm::vec3 e0( ( float ) box.dimX * 0.5f, 0.0f, 0.0f );
		glm::vec3 e1( 0.0f, ( float ) box.dimY * 0.5f, 0.0f );

		GU_ImmBegin( GL_LINE_LOOP, v, camPtr->ViewData().clipTransform );

		guImmPosList_t p =
		{
			s - e0 - e1,
			s - e0 + e1,
			s + e0 + e1,
			s + e0 - e1
		};
		GU_ImmLoad( p, box.color );
		GU_ImmEnd();
	} );

}

