#include "test_tessellation.h"
#include "../shader.h"
#include "../glutil.h"
#include <array>

static int colorIndex = 0;

static std::array< glm::vec4, 3 > colorTable = 
{
	glm::vec4( 1.0f, 0.0f, 0.0f, 1.0f ),
	glm::vec4( 0.0f, 1.0f, 0.0f, 1.0f ),
	glm::vec4( 0.0f, 0.0f, 1.0f, 1.0f )
};

static int vcount = 0;

static tessVert_t GenVertex( const glm::vec3& v, const tessellateInfo_t& info ) 
{
	tessVert_t vertex = 
	{
		v,
		colorTable[ colorIndex ]
	};

	vcount++;

	if ( vcount % 3 == 0 )
	{
		colorIndex = ( colorIndex + 1 ) % 3;
	}

	return vertex;
}

//----------------------------------------------------------------

TessTri::TessTri( const glm::mat3& verts )
	: modelTransform( 1.0f )
{
	const glm::vec4 color( 1.0f );

	mainVertices = 
	{
		{ verts[ 0 ], color },
		{ verts[ 1 ], color },
		{ verts[ 2 ], color }
	};

	glm::vec3 normal = glm::cross( mainVertices[ 1 ].position - mainVertices[ 0 ].position, mainVertices[ 2 ].position - mainVertices[ 1 ].position );

	TessellateTri< tessVert_t >( tessVertices, GenVertex, 16.0f, mainVertices[ 0 ].position, mainVertices[ 1 ].position, mainVertices[ 2 ].position, normal );

	auto LLoadLayout = []( void ) 
	{
		GL_CHECK( glEnableVertexAttribArray( 0 ) );
		GL_CHECK( glEnableVertexAttribArray( 1 ) );

		GL_CHECK( glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, sizeof( tessVert_t ), ATTRIB_OFFSET( tessVert_t, position ) ) );
		GL_CHECK( glVertexAttribPointer( 1, 4, GL_FLOAT, GL_FALSE, sizeof( tessVert_t ), ATTRIB_OFFSET( tessVert_t, color ) ) );
	};

	GL_CHECK( glGenVertexArrays( 2, vaos ) );
	GL_CHECK( glGenBuffers( 2, vbos ) );

	GL_CHECK( glBindVertexArray( vaos[ 0 ] ) );
	GL_CHECK( glBindBuffer( GL_ARRAY_BUFFER, vbos[ 0 ] ) );
	GL_CHECK( glBufferData( GL_ARRAY_BUFFER, sizeof( tessVert_t ) * mainVertices.size(), &mainVertices[ 0 ], GL_STATIC_DRAW ) );
	LLoadLayout();

	GL_CHECK( glBindVertexArray( vaos[ 1 ] ) );
	GL_CHECK( glBindBuffer( GL_ARRAY_BUFFER, vbos[ 1 ] ) );
	GL_CHECK( glBufferData( GL_ARRAY_BUFFER, sizeof( tessVert_t ) * tessVertices.size(), &tessVertices[ 0 ], GL_STATIC_DRAW ) );
	LLoadLayout();

	GL_CHECK( glBindBuffer( GL_ARRAY_BUFFER, 0 ) );
	GL_CHECK( glBindVertexArray( 0 ) );
}

TessTri::~TessTri( void )
{
	GL_CHECK( glBindVertexArray( 0 ) );
	GL_CHECK( glBindBuffer( GL_ARRAY_BUFFER, 0 ) );

	GL_CHECK( glDeleteVertexArrays( 2, vaos ) );
	GL_CHECK( glDeleteBuffers( 2, vbos ) );
}

void TessTri::Render( const glm::mat4& viewTransform, GLuint location )
{
	GL_CHECK( glUniformMatrix4fv( location, 1, GL_FALSE, glm::value_ptr( viewTransform * modelTransform ) ) );

	//GL_CHECK( glPolygonMode( GL_FRONT_AND_BACK, GL_FILL ) );
	GL_CHECK( glBindVertexArray( vaos[ 0 ] ) );
	GL_CHECK( glDrawArrays( GL_TRIANGLES, 0, 3 ) );
	
	//GL_CHECK( glPolygonMode( GL_FRONT_AND_BACK, GL_LINE ) );
	GL_CHECK( glBindVertexArray( vaos[ 1 ] ) );
	GL_CHECK( glDrawArrays( GL_TRIANGLE_STRIP, 0, tessVertices.size() ) );

	GL_CHECK( glBindVertexArray( 0 ) );
}

//----------------------------------------------------------------

TessTest::TessTest( void )
	:	Test( 1366, 768 ),
		program( 0 ),
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

	GLuint shaders[] = 
	{
		 CompileShaderSource( vertexShaderSrc, GL_VERTEX_SHADER ),
		 CompileShaderSource( fragmentShaderSrc, GL_FRAGMENT_SHADER )
	};

	program = LinkProgram( shaders, 2 );

	GL_CHECK( modelToViewLoc = glGetUniformLocation( program, "modelToView" ) );
	GL_CHECK( viewToClipLoc = glGetUniformLocation( program, "viewToClip" ) );

	GL_CHECK( glClearColor( 0.0f, 0.0f, 0.0f, 1.0f ) );

	camera->SetPerspective( glm::radians( 60.0f ), 16.0f / 9.0f, 0.1f, 10000.0f );
	GL_CHECK( glProgramUniformMatrix4fv( program, viewToClipLoc, 1, GL_FALSE, glm::value_ptr( camera->ViewData().clipTransform ) ) );

	GL_CHECK( glPointSize( 10.0f ) );

	TessTri* one = new TessTri( 
		glm::mat3( 
			1000.0f, 0.0f, 0.0f,
			0.0f, 1000.0f, 0.0f,
			0.0f, 0.0f, 1000.0f 
		) 
	);

	tris.push_back( one );

	TessTri* two = new TessTri(
		glm::mat3(
			600.0f, 0.0f, 0.0f,
			0.0f, 400.0f, 0.0f,
			0.0f, 0.0f, 200.0f 
		)
	);

	two->modelTransform = glm::translate( glm::mat4( 1.0f ), glm::vec3( 1500.0f, 0.0f, 0.0f ) );

	tris.push_back( two );
}

void TessTest::Run( void )
{
	camera->Update();

	const viewParams_t& view = camera->ViewData();

	GL_CHECK( glUseProgram( program ) );
	
	for ( TessTri* tri: tris )
		tri->Render( view.transform, modelToViewLoc );

	//GL_CHECK( glUseProgram( 0 ) );
	
	ImPrep( view.transform, view.clipTransform );
	ImDrawAxes( 1000.0f );

	/*
	glBegin( GL_POINTS );
	glColor3f( 1.0f, 0.0f, 0.0f );
	glVertex3fv( glm::value_ptr( mainVertices[ 0 ].position ) );

	glColor3f( 0.0f, 1.0f, 0.0f );
	glVertex3fv( glm::value_ptr( mainVertices[ 1 ].position ) );

	glColor3f( 0.0f, 0.0f, 1.0f );
	glVertex3fv( glm::value_ptr( mainVertices[ 2 ].position ) );
	glEnd();*/
}