#pragma once

#include "test.h"

struct tessVert_t
{
	glm::vec3 position;
	glm::vec4 color;
};

class TessTri
{
private:
	GLuint vbos[ 2 ];
	GLuint vaos[ 2 ];

	std::vector< tessVert_t > mainVertices;
	std::vector< tessVert_t > tessVertices;

public:
	glm::mat4 modelTransform;

	TessTri( const glm::mat3& verts );

	~TessTri( void );

	void Render( const glm::mat4& viewTransform, GLuint location );
};

class TessTest : public Test
{
private:
	
	GLuint program;

	GLint modelToViewLoc;
	GLint viewToClipLoc;

	std::vector< TessTri* > tris;

	InputCamera* camera; 

public:
	TessTest( void );
	
	~TessTest( void );

	void Load( void );
	
	void Run( void );
};