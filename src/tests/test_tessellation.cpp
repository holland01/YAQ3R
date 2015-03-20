#include "test_tessellation.h"
#include "../shader.h"
#include "../glutil.h"
#include "../q3bsp.h"
#include <array>
#include <random>
#include <cmath>
#include <iomanip>

static int colorIndex = 0;

static std::array< glm::vec4, 3 > colorTable = 
{
	glm::vec4( 1.0f, 0.0f, 0.0f, 1.0f ),
	glm::vec4( 0.0f, 1.0f, 0.0f, 1.0f ),
	glm::vec4( 0.0f, 0.0f, 1.0f, 1.0f )
};

static int vcount = 0;

//----------------------------------------------------------------

Program::Program( const char* vertexShader, const char* fragmentShader )
	: program( 0 )
{
	GLuint shaders[] = 
	{
		CompileShaderSource( vertexShader, GL_VERTEX_SHADER ),
		CompileShaderSource( fragmentShader, GL_FRAGMENT_SHADER )
	};

	program = LinkProgram( shaders, 2 );

	AddUnif( "modelToView" );
	AddUnif( "viewToClip" );
}

Program::~Program( void )
{
	Release();
	GL_CHECK( glDeleteProgram( program ) );
}

void Program::AddUnif( const std::string& name )
{
	GL_CHECK( uniforms[ name ] = glGetUniformLocation( program, name.c_str() ) ); 
}

void Program::LoadMatrix( const std::string& name, const glm::mat4& t )
{
	GL_CHECK( glProgramUniformMatrix4fv( program, uniforms[ name ], 1, GL_FALSE, glm::value_ptr( t ) ) );
}

void Program::LoadVec4( const std::string& name, const glm::vec4& v )
{
	GL_CHECK( glProgramUniform4fv( program, uniforms[ name ], 1, glm::value_ptr( v ) ) );
}

void Program::Bind( void )
{
	GL_CHECK( glUseProgram( program ) );
}

void Program::Release( void )
{
	GL_CHECK( glUseProgram( 0 ) );
}

//----------------------------------------------------------------

TessTri::TessTri( const glm::mat3& verts )
	: modelTransform( 1.0f )
{
	auto LGenVertex = [ &verts ]( int index ) -> bspVertex_t
	{
		bspVertex_t v = 
		{
			verts[ index ],
			{ 
				glm::vec2( 0.0f ),
				glm::vec2( 0.0f )
			},
			glm::vec3( 0.0f ),

			{
				255, 
				255, 
				255,
				255
			}
		};

		return v;
	};

	mainVertices = 
	{
		LGenVertex( 0 ),
		LGenVertex( 1 ),
		LGenVertex( 2 )
	};

	TessellateTri( tessVertices, tessIndices, 16.0f, mainVertices[ 0 ], mainVertices[ 1 ], mainVertices[ 2 ] );

	auto LLoadLayout = []( GLuint vao, GLuint vbo, bool hasColor, bool writeData, const std::vector< bspVertex_t >& vertexData ) 
	{
		GL_CHECK( glBindVertexArray( vao ) );
		GL_CHECK( glBindBuffer( GL_ARRAY_BUFFER, vbo ) );

		if ( writeData )
			GL_CHECK( glBufferData( GL_ARRAY_BUFFER, sizeof( bspVertex_t ) * vertexData.size(), &vertexData[ 0 ], GL_STATIC_DRAW ) );

		GL_CHECK( glEnableVertexAttribArray( 0 ) );
		GL_CHECK( glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, sizeof( bspVertex_t ), ATTRIB_OFFSET( bspVertex_t, position ) ) );
		
		if ( hasColor )
		{
			GL_CHECK( glEnableVertexAttribArray( 1 ) );
			GL_CHECK( glVertexAttribPointer( 1, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof( bspVertex_t ), ATTRIB_OFFSET( bspVertex_t, color ) ) );
		}
	};

	GL_CHECK( glGenVertexArrays( 3, vaos ) );
	GL_CHECK( glGenBuffers( 3, vbos ) );

	LLoadLayout( vaos[ 0 ], vbos[ 0 ], true, true, mainVertices );
	LLoadLayout( vaos[ 1 ], vbos[ 1 ], true, true, tessVertices );
	LLoadLayout( vaos[ 2 ], vbos[ 1 ], false, false, tessVertices );

	GL_CHECK( glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, vbos[ 2 ] ) );
	GL_CHECK( glBufferData( GL_ELEMENT_ARRAY_BUFFER, sizeof( triangle_t ) * tessIndices.size(), &tessIndices[ 0 ].indices[ 0 ], GL_STATIC_DRAW ) );
	GL_CHECK( glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 ) );

	GL_CHECK( glBindBuffer( GL_ARRAY_BUFFER, 0 ) );
	GL_CHECK( glBindVertexArray( 0 ) );
}

TessTri::~TessTri( void )
{
	GL_CHECK( glBindVertexArray( 0 ) );
	GL_CHECK( glBindBuffer( GL_ARRAY_BUFFER, 0 ) );
	GL_CHECK( glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 ) );

	GL_CHECK( glDeleteVertexArrays( 3, vaos ) );
	GL_CHECK( glDeleteBuffers( 3, vbos ) );
}

void TessTri::Render( int tessVaoIndex, const std::unique_ptr< Program >& program, const viewParams_t& view )
{
	program->LoadMatrix( "modelToView", view.transform * modelTransform );
	program->Bind();

	switch ( tessVaoIndex )
	{
		case 1:
		{
			GL_CHECK( glPolygonMode( GL_FRONT_AND_BACK, GL_LINE ) );
			GL_CHECK( glBindVertexArray( vaos[ 0 ] ) );
			GL_CHECK( glDrawArrays( GL_TRIANGLES, 0, 3 ) );
			GL_CHECK( glPolygonMode( GL_FRONT_AND_BACK, GL_FILL ) );
		}
		break;

		case 2:
		{
			GL_CHECK( glPolygonMode( GL_FRONT_AND_BACK, GL_LINE ) );
		}
		break;
	}

	GL_CHECK( glBindVertexArray( vaos[ tessVaoIndex ] ) );

	GL_CHECK( glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, vbos[ 2 ] ) );
	
	GL_CHECK( glDrawElements( GL_TRIANGLES, tessIndices.size() * 3, GL_UNSIGNED_INT, NULL ) );
	GL_CHECK( glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 ) );

	GL_CHECK( glBindVertexArray( 0 ) );

	program->Release();

	ImPrep( view.transform * modelTransform, view.clipTransform );
	ImDrawAxes( 1000.0f );

	glBegin( GL_POINTS );
		glColor3f( 1.0f, 0.0f, 0.0f );
		glVertex3fv( glm::value_ptr( mainVertices[ 0 ].position ) );

		glColor3f( 0.0f, 1.0f, 0.0f );
		glVertex3fv( glm::value_ptr( mainVertices[ 1 ].position ) );

		glColor3f( 0.0f, 0.0f, 1.0f );
		glVertex3fv( glm::value_ptr( mainVertices[ 2 ].position ) );
	glEnd();
}

//----------------------------------------------------------------

TessTest::TessTest( void )
	:	Test( 1366, 768 ),
		camera( nullptr )
{
}

TessTest::~TessTest( void )
{
	for ( auto i = tris.begin(); i != tris.end(); ++i )
	{
		if ( *i )
			delete *i;
	}
}

void TessTest::Load( void )
{
	bool good = Test::Load( "Tessellation" );
	assert( good );

	camera = new InputCamera();
	camPtr = camera;

	{
		const char* vertexShaderSrc = 
			"#version 420\n" \
			"layout(location = 0) in vec3 position;\n"	\
			"layout(location = 1) in vec4 color;\n"	\
			"uniform mat4 modelToView;\n" \
			"uniform mat4 viewToClip;\n" \
			"out vec4 frag_Color;\n" \
			"void main(void) {\n" \
			"   vec4 clipPos = viewToClip * modelToView * vec4( position, 1.0 );\n"	\
			"   gl_Position = clipPos;\n" \
			"   frag_Color = color;\n" \
			"}\n";

		const char* fragmentShaderSrc = 
			"#version 420\n" \
			"in vec4 frag_Color;\n" \
			"out vec4 fragment;\n" \
			"void main(void) {\n" \
			"   fragment = frag_Color;\n" \
			"}\n";

		fillProgram.reset( new Program( vertexShaderSrc, fragmentShaderSrc ) );
	}

	{
		const char* vertexShaderSrc = 
			"#version 420\n" \
			"layout(location = 0) in vec3 position;\n"	\
			"uniform mat4 modelToView;\n" \
			"uniform mat4 viewToClip;\n" \
			"void main(void) {\n" \
			"   vec4 clipPos = viewToClip * modelToView * vec4( position, 1.0 );\n"	\
			"   gl_Position = clipPos;\n" \
			"}\n";

		const char* fragmentShaderSrc = 
			"#version 420\n" \
			"uniform vec4 frag_Color;\n" \
			"out vec4 fragment;\n" \
			"void main(void) {\n" \
			"   fragment = frag_Color;\n" \
			"}\n";

		lineProgram.reset( new Program( vertexShaderSrc, fragmentShaderSrc ) );
		lineProgram->AddUnif( "frag_Color" );
	}

	GL_CHECK( glClearColor( 0.0f, 0.0f, 0.0f, 1.0f ) );

	camera->SetPerspective( glm::radians( 60.0f ), 16.0f / 9.0f, 0.1f, 10000.0f );
	fillProgram->LoadMatrix( "viewToClip", camera->ViewData().clipTransform );
	lineProgram->LoadMatrix( "viewToClip", camera->ViewData().clipTransform );

	GL_CHECK( glPointSize( 10.0f ) );
	GL_CHECK( glEnable( GL_DEPTH_TEST ) );
	GL_CHECK( glDepthFunc( GL_LEQUAL ) );
	GL_CHECK( glClearDepth( 1.0f ) );

	std::random_device rd;
	std::mt19937 e( rd() );
	std::uniform_real_distribution< float > dist( 0.0f, 1000.0f );

	for ( uint32_t i = 0; i < 5; ++i )
	{
		glm::vec3 a( dist( e ), 0.0f, 0.0f );
		glm::vec3 b( 0.0f, dist( e ), 0.0f );
		glm::vec3 c( 0.0f, 0.0f, dist( e ) );

		glm::mat3 v( a, b, c );

		TessTri* t =  new TessTri( v );

		t->modelTransform = glm::translate( glm::mat4( 1.0f ), glm::vec3( i * dist( e ), i * dist( e ), i * dist( e ) ) );

		tris.push_back( t );
	}
}

void TessTest::Run( void )
{
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	camera->Update();

	const viewParams_t& view = camera->ViewData();

	for ( TessTri* tri: tris )
	{
		tri->Render( 1, fillProgram, view );

		lineProgram->LoadVec4( "frag_Color", glm::vec4( 1.0f, 0.0f, 0.0f, 1.0f ) );
		tri->Render( 2, lineProgram, view );
	}	
}