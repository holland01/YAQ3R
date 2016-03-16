#include "test_textures.h"
#include "io.h"
#include "renderer/util.h"

TTextureTest::TTextureTest( void )
	: Test( 1366, 768, false ),
	  prog( nullptr ),
	  camera( new InputCamera() )
{
	this->camPtr = camera.get();
}

TTextureTest::~TTextureTest( void )
{
}

void TTextureTest::SetupVertexData( void )
{

}

void TTextureTest::SetupProgram( void )
{

}

void TTextureTest::Run( void )
{
	camera->Update();

	GBindTexture( texture );

	prog->LoadMat4( "modelToView", camera->ViewData().transform );

	prog->Bind();
	GBindVertexBuffer( vbo );

	GL_CHECK( glDrawArrays( GL_TRIANGLE_STRIP, 0, 4 ) );

	GReleaseVertexBuffer();

	prog->Release();
}

void TTextureTest::Load( void )
{
	if ( !Test::Load( "texture atlas testing" ) )
	{
		MLOG_ERROR( "failure" );
		return;
	}

	GEnableDepthBuffer();

	const std::string vertex = R"(
		in vec3 position;
		in vec2 tex0;

		uniform mat4 modelToView;
		uniform mat4 viewToClip;

		smooth out vec2 frag_TexCoords;

		void main(void)
		{
			gl_Position = viewToClip * modelToView * vec4(position, 1.0);
			frag_TexCoords = tex0;
		}
	)";

	const std::string fragment = R"(
		smooth in vec2 frag_TexCoords;

		//uniform vec4 imageTransform;
		//uniform vec2 imageScaleRatio;
		uniform sampler2D sampler;

		out vec4 out_Color;

		void main(void)
		{
			vec2 st = frag_TexCoords; //frag_TexCoords * imageTransform.zw * imageScaleRatio + imageTransform.xy;

			out_Color = texture( sampler, st );
		}
	)";

	prog.reset( new Program( vertex, fragment,
		{ "modelToView", "viewToClip",
							//"imageTransform",
							//"imageScaleRatio",
							"sampler"
		},
		{ "position", "tex0" } ) );

	camera->SetPerspective( 45.0f, ( float ) this->width, ( float ) this->height, 1.0f, 1000.0f );

	prog->LoadInt( "sampler", 0 );
	prog->LoadMat4( "viewToClip", camera->ViewData().clipTransform );

	float x = 512.0f;
	float y = 4096.0f;

	std::vector< glm::vec3 > vertices =
	{
		glm::vec3( x, 0.0f, 0.0f ),
		glm::vec3( x, y, 0.0f ),
		glm::vec3( 0.0f, 0.0f, 0.0f ),
		glm::vec3( 0.0f, y, 0.0f ),
	};

	std::vector< glm::vec2 > texCoords =
	{
		glm::vec2( 1.0f, 0.0f ),
		glm::vec2( 1.0f, 1.0f ),
		glm::vec2( 0.0f, 0.0f ),
		glm::vec2( 0.0f, 1.0f )
	};

	camera->SetViewOrigin( glm::vec3( 0.0f, 0.0f, 3.0f ) );

	vbo = GMakeVertexBuffer( vertices, texCoords );

	GL_CHECK( glDisable( GL_CULL_FACE ) );

	GBindVertexBuffer( vbo );
	prog->LoadDefaultAttribProfiles();
	GReleaseVertexBuffer();

	GL_CHECK( glClearColor( 1.0f, 0.0f, 0.0f, 1.0f ) );

	map.Read( "asset/stockmaps/maps/q3tourney2.bsp", 1 );

	sampler = GMakeSampler();
	texture = GU_LoadShaderTextures( map, sampler );

	MLOG_INFO( "hrhr" );
}
