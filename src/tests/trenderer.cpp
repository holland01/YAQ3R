#include "trenderer.h"
#include "../glutil.h"

TRenderer::TRenderer( void )
	: Test( 1920, 1080, false ),
	  renderer( nullptr ),
	  mapFilepath( "asset/stockmaps/maps/Railgun_Arena.bsp" )
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
	if ( !Test::Load( "I am a floating camera" ) )
	{
		MLOG_ERROR( "Could not initialize the necessary rendering prerequisites." );
		return;
	}

	renderer = new BSPRenderer( ( float ) width, ( float ) height );
	renderer->Prep();
	renderer->Load( mapFilepath );
	camPtr = renderer->camera;
}
