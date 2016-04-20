#include "test_textures.h"
#include "io.h"
#include "renderer/util.h"

TTextureTest::TTextureTest( void )
	: Test( 1366, 768, false ),
	  atlasProg( nullptr ),
	  camera( new InputCamera() ),
	  drawAtlas( true ), drawGrid( false ),
	  currImageKey( 0 ), currGridKey( 0 ),
	  numGrids( 0 )
{
	this->camPtr = camera.get();
}

TTextureTest::~TTextureTest( void )
{
}

void TTextureTest::Load( void )
{
	if ( !Test::Load( "texture atlas testing" ) )
	{
		MLOG_ERROR( "failure" );
		return;
	}

	GEnableDepthBuffer();

	camera->SetViewOrigin( glm::vec3( 0.0f, 0.0f, 3.0f ) );
	camera->moveStep = 1.0f;
	camera->SetPerspective( 45.0f, ( float ) this->width, ( float ) this->height, 1.0f, 10000.0f );

	std::string vertex = R"(
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

	std::string fragment = R"(
		smooth in vec2 frag_TexCoords;
		uniform sampler2D sampler0;

		out vec4 out_Color;

		void main(void)
		{
			vec2 st = frag_TexCoords; //frag_TexCoords * imageTransform.zw * imageScaleRatio + imageTransform.xy;

			out_Color = texture( sampler0, st );
		}
	)";

	atlasProg.reset( MakeProgram( vertex, fragment ) );

	vertex = R"(
		in vec3 position;
		in vec2 tex0;

		uniform mat4 modelToView;
		uniform mat4 viewToClip;

		smooth out vec2 frag_TexCoords;

		void main(void)
		{
			gl_Position = viewToClip * modelToView * vec4(position, 1.0);
			frag_TexCoords = tex0; //* imageTransform.zw * imageScaleRatio + imageTransform.xy;
		}
	)";

	fragment = R"(
		smooth in vec2 frag_TexCoords;

		uniform sampler2D sampler0;
		uniform vec4 imageTransform;
		uniform vec2 imageScaleRatio;

		out vec4 out_Color;

		void main(void)
		{
			vec2 st = frag_TexCoords * imageScaleRatio * imageTransform.zw + imageTransform.xy;
			out_Color = texture( sampler0, st );
		}
	)";

	textureProg.reset( MakeProgram( vertex, fragment, { "imageTransform", "imageScaleRatio" } ) );

	map.Read( "asset/stockmaps/maps/q3dm2.bsp", 1 );

	sampler = GMakeSampler();
	texture = GU_LoadMainTextures( map, sampler );
	imageKeys = GTextureImageKeys( texture );
	numGrids = GTextureGridCount( texture );

	MLOG_ASSERT( imageKeys.size() > 0, "imageKeys member is empty..." );

	atlasQuad = MakeQuadVbo( GTextureMegaWidth( texture ), GTextureMegaHeight( texture ) );
	textureQuad = MakeQuadVbo( 1.0f, 1.0f );

	GL_CHECK( glDisable( GL_CULL_FACE ) );

	GBindVertexBuffer( atlasQuad );
	atlasProg->LoadDefaultAttribProfiles();
	GReleaseVertexBuffer();

	GL_CHECK( glClearColor( 1.0f, 0.0f, 0.0f, 1.0f ) );

	//MLOG_INFO( "hrhr" );
}

static void DecKey( gTextureImageKey_t& k, const size_t max )
{
	if ( k == 0 )
	{
		k = max - 1;
	}
	else
	{
		k--;
	}
}

static void IncKey( gTextureImageKey_t& k, const size_t max )
{
	k = ( k + 1 ) % max;
}

void TTextureTest::OnInputEvent( SDL_Event* e )
{
	Test::OnInputEvent( e );

	if ( e->type == SDL_KEYDOWN )
	{
		switch ( e->key.keysym.sym )
		{
			case SDLK_g:
				drawGrid = !drawGrid;
				break;
			case SDLK_UP:
				drawAtlas = !drawAtlas;
				break;
			case SDLK_RIGHT:
				if ( drawGrid )
				{
					IncKey( currGridKey, numGrids );
				}
				else
				{
					IncKey( currImageKey, imageKeys.size() );
				}
				break;
			case SDLK_LEFT:
				if ( drawGrid )
				{
					DecKey( currGridKey, numGrids );
				}
				else
				{
					DecKey( currImageKey, imageKeys.size() );
				}
				break;
		}
	}
}

Program * TTextureTest::MakeProgram( const std::string& vertex,
									 const std::string& fragment,
									 const std::vector< std::string >& additionalUnifs )
{
	std::vector< std::string > unifs = { "modelToView","viewToClip",
										 "sampler0" };

	unifs.insert( unifs.end(), additionalUnifs.begin(), additionalUnifs.end() );

	Program* p = new Program( vertex, fragment,
		unifs,
		{ "position", "tex0" } );

	p->LoadInt( "sampler0", 0 );
	p->LoadMat4( "viewToClip", camera->ViewData().clipTransform );

	return p;
}

gVertexBufferHandle_t TTextureTest::MakeQuadVbo( float width, float height, float s, float t )
{
	std::vector< glm::vec3 > vertices =
	{
		glm::vec3( width, 0.0f, 0.0f ),
		glm::vec3( width, height, 0.0f ),
		glm::vec3( 0.0f, 0.0f, 0.0f ),
		glm::vec3( 0.0f, height, 0.0f ),
	};

	std::vector< glm::vec2 > texCoords =
	{
		glm::vec2( s,	 0.0f ),
		glm::vec2( s,		t ),
		glm::vec2( 0.0f, 0.0f ),
		glm::vec2( 0.0f,	t )
	};

	return GMakeVertexBuffer( vertices, texCoords );
}

void TTextureTest::Draw( Program& program, gVertexBufferHandle_t vbo,
		const glm::mat4& model )
{
	program.LoadMat4( "modelToView",
		camera->ViewData().transform * model );

	program.Bind();
	GBindVertexBuffer( vbo );
	program.LoadDefaultAttribProfiles();

	GL_CHECK( glDrawArrays( GL_TRIANGLE_STRIP, 0, 4 ) );

	GReleaseVertexBuffer();
	program.Release();
}

void TTextureTest::Run( void )
{
	camera->Update();

	if ( drawAtlas )
	{
		if ( drawGrid )
		{
			GBindGrid( texture, currGridKey );
		}
		else
		{
			GBindTexture( texture );
		}

		Draw( *atlasProg, atlasQuad );
	}
	else
	{
		const gTextureImage_t& img = GTextureImage( texture, imageKeys[ currImageKey ] );

		glm::mat4 model( glm::scale( glm::mat4( 1.0f ), glm::vec3( img.dims, 1.0f ) ) );

		GU_SetupTexParams( *textureProg, nullptr, texture, imageKeys[ currImageKey ], 0 );
		Draw( *textureProg, textureQuad, model );
	}
}
