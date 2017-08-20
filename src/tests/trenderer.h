#pragma once

#include "test.h"
#include "../renderer.h"

class TRenderer : public Test
{
public:
	glm::vec3 lightSamplerPos;

	float moveRateChangeRate;

	void LogPeriodically( void ) const;

	virtual void Run( void );

	std::unique_ptr< BSPRenderer > renderer;

	TRenderer( const char* pathString, onFinishEvent_t mapReadFinish );

	TRenderer( onFinishEvent_t mapReadFinish );

	TRenderer( const std::string& mapFilepath );

	virtual ~TRenderer( void );

	void 	Load( void );

	bool 	OnInputEvent( SDL_Event* e );

	float 	GetTargetFPS( void ) const { return TEST_FPS_60; } // 60 FPS
};

class TRendererIsolatedTest : public TRenderer
{
public:
	TRendererIsolatedTest( void );
	~TRendererIsolatedTest( void );

	std::unique_ptr< RenderBase > isolatedRenderer;

	void Run( void ) override;
};
