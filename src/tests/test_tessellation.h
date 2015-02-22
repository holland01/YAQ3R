#pragma once

#include "test.h"

struct tessVert_t
{
	glm::vec3 position;
	glm::vec4 color;
};

class TessTest : public Test
{
private:
	GLuint vbos[ 2 ];

	GLuint vaos[ 2 ];
	GLuint program;

	GLint modelToViewLoc;
	GLint viewToClipLoc;

	std::vector< tessVert_t > mainVertices;
	std::vector< tessVert_t > tessVertices;

	InputCamera* camera; 

public:
	TessTest( void );
	
	~TessTest( void );

	void Load( void );
	
	void Run( void );
};