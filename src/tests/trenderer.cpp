#include "trenderer.h"
#include "../glutil.h"

static const char* gTitle = "I am a floating camera";

// Below strings are commented to prevent compiler complaint
static const char* railgunArena = "asset/stockmaps/maps/Railgun_Arena.bsp"; //"asset/quake/railgun_arena/maps/Railgun_Arena.bsp";
/*
static const char* egyptTemple = "asset/quake/aty3dm1v2/aty3dm1v2/maps/aty3dm1v2.bsp";
static const char* neDuel = "asset/quake/ne_duel/maps/ne_duel.bsp";
static const char* dm1 = "asset/stockmaps/maps/q3dm15.bsp";
static const char* tourney = "asset/stockmaps/maps/q3tourney2.bsp";
*/

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
