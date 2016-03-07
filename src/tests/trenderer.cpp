#include "trenderer.h"
#include "../glutil.h"

static const char* gTitle = "I am a floating camera";

// Below strings are commented to prevent compiler complaint
static const char* railgunArena = "asset/stockmaps/maps/Railgun_Arena.bsp";

TRenderer::TRenderer( void )
	: Test( 1920, 1080, false ),
	  renderer( nullptr ),
	  mapFilepath( railgunArena )
{
}

TRenderer::~TRenderer( void )
{
	camPtr = nullptr;
	delete renderer;
}

void TRenderer::Run( void )
{
	renderer->Update( deltaTime );
	renderer->Render();
}

void TRenderer::Load( void )
{
	if ( !Test::Load( gTitle ) )
	{
		MLOG_ERROR( "Could not initialize the necessary rendering prerequisites." );
		return;
	}

	renderer = new BSPRenderer( ( float ) width, ( float ) height );
	renderer->Prep();
	renderer->Load( mapFilepath );
	camPtr = renderer->camera;
}
