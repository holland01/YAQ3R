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
    if ( vao )
    {
        GL_CHECK( glDeleteVertexArrays( 1, &vao ) );
    }
}

void TTextureTest::Run( void )
{
    camera->Update();

    prog->LoadVec4( "imageTransform", imageTransform );
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

    GL_CHECK( glGenVertexArrays( 1, &vao ) );
    GL_CHECK( glBindVertexArray( vao ) );

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
        uniform sampler2D sampler;

        out vec4 out_Color;

        void main(void)
        {
            vec2 st = frag_TexCoords * imageTransform.zw + imageTransform.xy;

            out_Color = texture( sampler, st );
        }
    )";

    prog.reset( new Program( vertex, fragment,
        { "modelToView", "viewToClip", "imageTransform", "sampler" },
        { "position", "tex0" },
            false ) );

    camera->SetPerspective( 45.0f, this->width, this->height, 0.01f, 10000.0f );

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
    prog->LoadAttribLayout();
    GReleaseVertexBuffer();

    GEnableDepthBuffer();

    GL_CHECK( glClearColor( 1.0f, 0.0f, 0.0f, 1.0f ) );

    std::vector< std::string > imagePaths =
    {
       // "asset/random/image/01.jpg",
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

    texture = GMakeTexture( imageInfo, 0 );
    imageTransform = GTextureImageDimensions( texture, 2 );
}

void TTextureTest::OnKeyPress( int key, int scancode, int action, int mods )
{
    Test::OnKeyPress( key, scancode, action, mods );
}
