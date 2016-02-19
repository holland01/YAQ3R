#include "test_textures.h"
#include "io.h"

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

void TTextureTest::Run( void )
{
	camera->Update();

	glm::vec4 imageTransform;

	imageTransform.x = currImage.stOffsetStart.x;
	imageTransform.y = currImage.stOffsetStart.y;
	imageTransform.z = invRowPitch.x;
	imageTransform.w = invRowPitch.y;

	glm::vec2 imageScaleRatio = currImage.imageScaleRatio;

	prog->LoadVec4( "imageTransform", imageTransform );
	prog->LoadVec2( "imageScaleRatio", imageScaleRatio );
	prog->LoadMat4( "modelToView", camera->ViewData().transform );
	prog->LoadInt( "sampler", 0 );
	prog->Bind();

	GBindTexture( texture );

	GBindVertexBuffer( vbo );

	GL_CHECK( glDrawArrays( GL_TRIANGLE_STRIP, 0, 4 ) );

	GReleaseVertexBuffer();
	GReleaseTexture( texture );

	prog->Release();
}

void TTextureTest::Load( void )
{
	if ( !Test::Load( "texture atlas testing" ) )
	{
		MLOG_ERROR( "failure" );
		return;
	}

	const std::string vertex = R"(
		#version 450
		layout(location = 0) in vec3 position;
		layout(location = 1) in vec2 tex0;

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
		#version 450
		smooth in vec2 frag_TexCoords;

		uniform vec4 imageTransform;
		uniform vec2 imageScaleRatio;
		uniform sampler2D sampler;

		out vec4 out_Color;

		void main(void)
		{
			vec2 st = frag_TexCoords * imageTransform.zw * imageScaleRatio + imageTransform.xy;

			out_Color = texture( sampler, st );
		}
	)";

	prog.reset( new Program( vertex, fragment,
		{ "modelToView", "viewToClip", "imageTransform", "imageScaleRatio", "sampler" },
		{ "position", "tex0" } ) );

	camera->SetPerspective( 45.0f, ( float ) this->width, ( float ) this->height, 0.01f, 10000.0f );

	prog->LoadMat4( "viewToClip", camera->ViewData().clipTransform );

	float size = 10.0f;

	std::vector< glm::vec3 > vertices =
	{
		glm::vec3( size, -size, 0.0f ),
		glm::vec3( size, size, 0.0f ),
		glm::vec3( -size, -size, 0.0f ),
		glm::vec3( -size, size, 0.0f ),
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

	GEnableDepthBuffer();

	GL_CHECK( glClearColor( 1.0f, 0.0f, 0.0f, 1.0f ) );

	std::vector< std::string > imagePaths =
	{
		"asset/random/image/01.png",
		"asset/random/image/02.png",
		"asset/random/image/03.png",
		"asset/random/image/04.png"
	};

	std::vector< gImageParams_t > imageInfo;

	for ( const std::string& s: imagePaths )
	{
		gImageParams_t image;
		bool res = GLoadImageFromFile( s, image );
		if ( res )
			imageInfo.push_back( image );
	}

	gSamplerHandle_t sampler = GMakeSampler();
	gTextureMakeParams_t makeParams( imageInfo, sampler );
	texture = GMakeTexture( makeParams, 0 );
	currImage = GTextureImage( texture, 3 );
	invRowPitch = GTextureInverseRowPitch( texture );
}
