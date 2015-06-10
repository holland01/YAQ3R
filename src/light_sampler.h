#pragma once

#include "common.h"
#include "glutil.h"

struct mapData_t;

class LightSampler
{
private:
	GLuint fbo;
	texture_t texAttach;

	glm::mat4 view, projection;

	Program program;

public:
	LightSampler( const mapData_t& data );
	~LightSampler( void );

	void Pass( void );
};