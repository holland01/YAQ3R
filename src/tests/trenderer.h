#pragma once

#include "test.h"
#include "../renderer.h"

class TRenderer : public Test
{
public:
	glm::vec3 lightSamplerPos;

	float moveRateChangeRate;

	void LogPeriodically( void ) const;

	virtual void Run( void ) override;

	std::unique_ptr< BSPRenderer > renderer;

	TRenderer( const char* pathString, onFinishEvent_t mapReadFinish );

	TRenderer( onFinishEvent_t mapReadFinish );

	TRenderer( const std::string& mapFilepath );

	virtual ~TRenderer( void );

	virtual void Load( void ) override;

	bool 	OnInputEvent( SDL_Event* e ) override;

	float 	GetTargetFPS( void ) const { return TEST_FPS_60; } // 60 FPS
};

class TRendererIsolatedTest : public TRenderer
{
public:
	TRendererIsolatedTest( void );

	~TRendererIsolatedTest( void );

	GLuint texture;

	std::unique_ptr< RenderBase > isolatedRenderer;

	void Load_Quad( void );

	void Run_QuadBsp( void );

	void Run_Quad( void );

	void Run_Skybox( void );

	virtual void Load( void ) override;

	void Run( void ) override;
};
