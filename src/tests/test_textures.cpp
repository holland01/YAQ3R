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

    GL_CHECK( glGenVertexArrays( 1, &vao ) );
    GL_CHECK( glBindVertexArray( vao ) );

    const std::string vertex = R"(
        #version 450
        layout(location = 0) in vec3 position;

        uniform mat4 modelToView;
        uniform mat4 viewToClip;

        void main(void)
        {
            gl_Position = viewToClip * modelToView * vec4(position, 1.0);
        }
    )";

    const std::string fragment = R"(
        #version 450
        out vec4 out_Color;

        void main(void)
        {
            out_Color = vec4(1.0);
        }
    )";

    prog.reset( new Program( vertex, fragment, { "modelToView", "viewToClip" }, { "position" }, false ) );

    camera->SetPerspective( 45.0f, this->width, this->height, 0.01f, 10000.0f );

    prog->LoadMat4( "viewToClip", camera->ViewData().clipTransform );

    float size = 10.0f;

    std::vector< glm::vec3 > vertices =
    {
        glm::vec3( size, -size, 0.0f ),
        glm::vec3( size, size, 0.0f ),
        glm::vec3( -size, -size, 0.0f ),
        glm::vec3( -size, size, 0.0f )
    };

    camera->SetViewOrigin( glm::vec3( 0.0f, 0.0f, 3.0f ) );

    vbo = GMakeVertexBuffer( vertices );

    GL_CHECK( glDisable( GL_CULL_FACE ) );

    GBindVertexBuffer( vbo );
    prog->LoadAttribLayout();
    GReleaseVertexBuffer();

    GEnableDepthBuffer();

    GL_CHECK( glClearColor( 1.0f, 0.0f, 0.0f, 1.0f ) );
}

void TTextureTest::OnKeyPress( int key, int scancode, int action, int mods )
{
    Test::OnKeyPress( key, scancode, action, mods );
}
