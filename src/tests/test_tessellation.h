#pragma once

#include "test.h"

struct triangle_t;

struct tessVert_t
{
	glm::vec3 position;
	glm::vec4 color;
};


bool operator == ( const tessVert_t& a, const tessVert_t& b );
bool operator != ( const tessVert_t& a, const tessVert_t& b );

class TessTri
{
private:
	GLuint vbos[ 3 ];
	GLuint vaos[ 2 ];

	std::vector< tessVert_t > mainVertices;
	std::vector< tessVert_t > tessVertices;
	std::vector< triangle_t > tessIndices;

public:
	glm::mat4 modelTransform;

	TessTri( const glm::mat3& verts );

	~TessTri( void );

	void Render( const viewParams_t& view, GLuint location );
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