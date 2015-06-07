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

	AddAttrib( "position" );
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

void Program::AddAttrib( const std::string& name )
{
	GL_CHECK( attribs[ name ] = glGetAttribLocation( program, name.c_str() ) );
}

void Program::LoadMatrix( const std::string& name, const glm::mat4& t )
{
	GL_CHECK( glProgramUniformMatrix4fv( program, uniforms[ name ], 1, GL_FALSE, glm::value_ptr( t ) ) );
}

void Program::LoadVec4( const std::string& name, const glm::vec4& v )
{
	GL_CHECK( glProgramUniform4fv( program, uniforms[ name ], 1, glm::value_ptr( v ) ) );
}

void Program::LoadInt( const std::string& name, int v )
{
	GL_CHECK( glProgramUniform1i( program, uniforms[ name ], v ) );
}

void Program::Bind( void )
{
	GL_CHECK( glUseProgram( program ) );
}

void Program::Release( void )
{
	GL_CHECK( glUseProgram( 0 ) );
}

enum 
{
	LAYOUT_WRITE_DATA = 0x1,
	LAYOUT_HAS_ATTRIB_COLOR = 0x2,
	LAYOUT_HAS_ATTRIB_UV = 0x4
};

//----------------------------------------------------------------

TessTri::TessTri( const TessTest* test, const std::array< glm::vec3, 4 >& verts )
    : texture( 0 ),
      sampler( 0 ),
      sharedTest( test ),
      modelTransform( 1.0f )
{
	{
		GL_CHECK( glGenTextures( 1, &texture ) );
		GL_CHECK( glGenSamplers( 1, &sampler ) );

		// TODO: reimpliment with new API
		///		bool test = LoadTextureFromFile( "asset/random/image/business_cat.jpg", texture, sampler, Q3LOAD_TEXTURE_SRGB, GL_CLAMP_TO_EDGE );
		assert( test );
	}

	auto LGenVertex = [ &verts ]( int index ) -> bspVertex_t
	{
		bspVertex_t v;

		v.position = verts[ index ];
		v.color[ 0 ] = v.color[ 1 ] = v.color[ 2 ] = v.color[ 3 ] = 255;

		return v;
	};

	auto LSetTexCoords = [ & ]( int which, float s, float t ) 
	{
		mainVertices[ which ].texCoords[ 0 ].s = mainVertices[ which ].texCoords[ 1 ].s = s;
		mainVertices[ which ].texCoords[ 0 ].t = mainVertices[ which ].texCoords[ 1 ].t = t;
	};

	mainVertices = 
	{
		LGenVertex( 0 ),
		LGenVertex( 1 ),
		LGenVertex( 2 ),
		LGenVertex( 3 ),
	};

	LSetTexCoords( 3, 1.0f, 1.0f );
	LSetTexCoords( 2, 0.0f, 0.0f );
	LSetTexCoords( 1, 0.0f, 1.0f );
	LSetTexCoords( 0, 1.0f, 0.0f );

	float f = glm::length( glm::cross( mainVertices[ 0 ].position, mainVertices[ 1 ].position ) ) / ( glm::length( mainVertices[ 0 ].position ) * glm::length( mainVertices[ 1 ].position ) );

	TessellateTri( tessVertices, tessIndices, 32.0f * f, 0.0f, mainVertices[ 0 ], mainVertices[ 1 ], mainVertices[ 2 ] );
	TessellateTri( tessVertices, tessIndices, 32.0f * f, 0.0f, mainVertices[ 0 ], mainVertices[ 3 ], mainVertices[ 1 ] );

	auto LLoadLayout = []( const std::unique_ptr< Program >& prog, GLuint vao, GLuint vbo, uint32_t flags, const std::vector< bspVertex_t >& vertexData ) 
	{
		GL_CHECK( glBindVertexArray( vao ) );
		GL_CHECK( glBindBuffer( GL_ARRAY_BUFFER, vbo ) );

		if ( ( flags & LAYOUT_WRITE_DATA ) != 0 )
			GL_CHECK( glBufferData( GL_ARRAY_BUFFER, sizeof( bspVertex_t ) * vertexData.size(), &vertexData[ 0 ], GL_STATIC_DRAW ) );

		GL_CHECK( glEnableVertexAttribArray( prog->attribs[ "position" ] ) );
		GL_CHECK( glVertexAttribPointer( prog->attribs[ "position" ], 3, GL_FLOAT, GL_FALSE, sizeof( bspVertex_t ), ATTRIB_OFFSET( bspVertex_t, position ) ) );
		
		if ( ( flags & LAYOUT_HAS_ATTRIB_COLOR ) != 0 )
		{
			GL_CHECK( glEnableVertexAttribArray( prog->attribs[ "color" ] ) );
			GL_CHECK( glVertexAttribPointer( prog->attribs[ "color" ], 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof( bspVertex_t ), ATTRIB_OFFSET( bspVertex_t, color ) ) );
		}

		if ( ( flags & LAYOUT_HAS_ATTRIB_UV ) != 0 )
		{
			GL_CHECK( glEnableVertexAttribArray( prog->attribs[ "uv" ] ) );
			GL_CHECK( glVertexAttribPointer( prog->attribs[ "uv" ], 2, GL_FLOAT, GL_FALSE, sizeof( bspVertex_t ), ATTRIB_OFFSET( bspVertex_t, texCoords[ 0 ] ) ) );
		}
	};

	GL_CHECK( glGenVertexArrays( 3, vaos ) );
	GL_CHECK( glGenBuffers( TESS_TEST_NUM_VBOS, vbos ) );

	LLoadLayout( test->fillProgram, vaos[ 0 ], vbos[ 0 ], LAYOUT_HAS_ATTRIB_COLOR | LAYOUT_HAS_ATTRIB_UV | LAYOUT_WRITE_DATA, mainVertices );
	LLoadLayout( test->fillProgram, vaos[ 1 ], vbos[ 1 ], LAYOUT_HAS_ATTRIB_COLOR | LAYOUT_HAS_ATTRIB_UV | LAYOUT_WRITE_DATA, tessVertices );
	LLoadLayout( test->lineProgram, vaos[ 2 ], vbos[ 1 ], 0, tessVertices );

	GL_CHECK( glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, vbos[ 2 ] ) );
	GL_CHECK( glBufferData( GL_ELEMENT_ARRAY_BUFFER, sizeof( triangle_t ) * tessIndices.size(), &tessIndices[ 0 ].indices[ 0 ], GL_STATIC_DRAW ) );

	GL_CHECK( glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, vbos[ 3 ] ) );
	GLuint mainVertIndices[] = 
	{
		0, 1, 2, 0, 3, 1
	};
	GL_CHECK( glBufferData( GL_ELEMENT_ARRAY_BUFFER, sizeof( mainVertIndices ), mainVertIndices, GL_STATIC_DRAW ) );
	
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
	GL_CHECK( glDeleteBuffers( TESS_TEST_NUM_VBOS, vbos ) );
}

void TessTri::Render( int tessVaoIndex, const std::unique_ptr< Program >& program, const viewParams_t& view )
{
	GL_CHECK( glBindTexture( GL_TEXTURE_2D, texture ) );
	GL_CHECK( glBindSampler( 0, sampler ) );

	program->LoadMatrix( "modelToView", view.transform * modelTransform );
	program->Bind();

	switch ( tessVaoIndex )
	{
		case 1:
		{
			//GL_CHECK( glPolygonMode( GL_FRONT_AND_BACK, GL_LINE ) );
			GL_CHECK( glBindVertexArray( vaos[ 0 ] ) );
			GL_CHECK( glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, vbos[ 3 ] ) );
			GL_CHECK( glDrawElements( GL_TRIANGLES, 6, GL_UNSIGNED_INT, NULL ) );
			GL_CHECK( glPolygonMode( GL_FRONT_AND_BACK, GL_FILL ) );
		}
		break;

		case 2:
		{
			GL_CHECK( glPolygonMode( GL_FRONT_AND_BACK, GL_LINE ) );
		}
		break;
	}

	/*
	GL_CHECK( glBindVertexArray( vaos[ tessVaoIndex ] ) );

	GL_CHECK( glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, vbos[ 2 ] ) );
	
	GL_CHECK( glDrawElements( GL_TRIANGLES, tessIndices.size() * 3, GL_UNSIGNED_INT, NULL ) );
	GL_CHECK( glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 ) );
	*/
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
			"layout(location = 1) in vec2 uv;\n" \
			"layout(location = 2) in vec4 color;\n"	\
			"uniform mat4 modelToView;\n" \
			"uniform mat4 viewToClip;\n" \
			"out vec4 frag_Color;\n" \
			"out vec2 frag_UV; \n" \
			"const vec4 gammaDecode = vec4( 2.2 );"
			"void main(void) {\n" \
			"   vec4 clipPos = viewToClip * modelToView * vec4( position, 1.0 );\n"	\
			"   gl_Position = clipPos;\n" \
			"   frag_Color = pow( color, gammaDecode );\n" \
			"   frag_UV = uv;\n" \
			"}\n";

		const char* fragmentShaderSrc = 
			"#version 420\n" \
			"in vec4 frag_Color;\n" \
			"in vec2 frag_UV;\n" \
			"uniform sampler2D texSampler;\n" \
			"out vec4 fragment;\n" \
			"void main(void) {\n" \
			"   fragment = pow( frag_Color * pow( texture( texSampler, frag_UV ), vec4( 2.2 ) ), vec4( 1.0 / 2.2 ) );\n" \
			"}\n";

		fillProgram.reset( new Program( vertexShaderSrc, fragmentShaderSrc ) );
		fillProgram->AddAttrib( "uv" );
		fillProgram->AddAttrib( "color" );
		fillProgram->AddUnif( "texSampler" );
		fillProgram->LoadInt( "texSampler", 0 );
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
		float base = dist( e );
		//std::uniform_real_distribution< float > vertexDist( base, base + 5.0f );

		glm::vec3 a( base, 0.0f, 0.0f );
		glm::vec3 b( 0.0f, base, 0.0f );
		glm::vec3 c( 0.0f, 0.0f, 0.0f );
		glm::vec3 d( a.x, b.y, 0.0f );

		glm::vec3 e1( glm::normalize( a - b ) );
		glm::vec3 e2( glm::normalize( c - b ) );
		glm::vec3 n( glm::normalize( glm::cross( e2, e1 ) ) );

		float dn = glm::dot( a, n );
		float distToPlane = glm::dot( d, n ) - dn;
		
		d = d - glm::normalize( n ) * distToPlane;

		std::array< glm::vec3, 4 > v = { a, b, c, d };

		TessTri* t = new TessTri( this, v );

		t->modelTransform = glm::translate( glm::mat4( 1.0f ), glm::vec3( i * dist( e ), i * dist( e ), i * dist( e ) ) );

		tris.push_back( t );
	}

	GL_CHECK( glActiveTexture( GL_TEXTURE0 ) );
}

void TessTest::Run( void )
{
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	camera->Update();

	const viewParams_t& view = camera->ViewData();

	for ( TessTri* tri: tris )
	{
		tri->Render( 1, fillProgram, view );

		//lineProgram->LoadVec4( "frag_Color", glm::vec4( 1.0f, 0.0f, 0.0f, 1.0f ) );

		//tri->Render( 2, lineProgram, view );
	}	

	GL_CHECK( glBindTexture( GL_TEXTURE_2D, 0 ) );
	GL_CHECK( glBindSampler( 0, 0 ) );
}
