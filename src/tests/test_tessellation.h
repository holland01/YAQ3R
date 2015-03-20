#pragma once

#include "test.h"
#include "../q3m_model.h"
#include <memory>

struct bspVertex_t;

struct Program
{
	GLuint program;
	std::map< std::string, GLint > uniforms; 

	Program( const char* vertexShader, const char* fragmentShader );
	~Program( void );

	void AddUnif( const std::string& name );

	void LoadMatrix( const std::string& name, const glm::mat4& t );
	void LoadVec4( const std::string& name, const glm::vec4& v );

	void Bind( void );
	void Release( void );
};

class TessTri
{
private:
	GLuint vbos[ 3 ];
	GLuint vaos[ 3 ];

	std::vector< bspVertex_t > mainVertices;
	std::vector< bspVertex_t > tessVertices;
	std::vector< triangle_t > tessIndices;

public:
	glm::mat4 modelTransform;

	TessTri( const glm::mat3& verts );

	~TessTri( void );

	void Render( int vaoIndex, const std::unique_ptr< Program >& program, const viewParams_t& view );
};

class TessTest : public Test
{
private:
	
	std::unique_ptr< Program > fillProgram, lineProgram;

	std::vector< TessTri* > tris;

	InputCamera* camera; 

public:
	TessTest( void );
	
	~TessTest( void );

	void Load( void );
	
	void Run( void );
};