#pragma once

#include "test.h"
#include "../bsp_data.h"
#include <memory>
#include <array>

//struct bspVertex_t;

class TessTest;

static const int TESS_TEST_NUM_VBOS = 4;

class TessTri
{
private:
	GLuint vbos[ TESS_TEST_NUM_VBOS ];
	GLuint vaos[ 3 ];
	GLuint texture, sampler;

	std::vector< bspVertex_t > mainVertices;
	std::vector< bspVertex_t > tessVertices;
	std::vector< triangle_t > tessIndices;

	const TessTest* sharedTest; 

public:
	glm::mat4 modelTransform;

	TessTri( const TessTest* test, const std::array< glm::vec3, 4 >& verts );

	~TessTri( void );

	void Render( int vaoIndex, const std::unique_ptr< Program >& program, const viewParams_t& view );
};

class TessTest : public Test
{
private:
	
	friend class TessTri;

	GLuint texture, sampler;

	std::unique_ptr< Program > fillProgram, lineProgram;

	std::vector< TessTri* > tris;

	InputCamera* camera; 

public:
	TessTest( void );
	
	~TessTest( void );

	void Load( void );
	
	void Run( void );
};