#include "tlighting.h"
#include "../log.h"
#include "../shader.h"

TLighting::TLighting( void )
    : Test( 1366, 768 ),
      camera( new Camera ),
      program( 0 )
{
    camPtr = camera;
}

TLighting::~TLighting( void )
{
    glDeleteVertexArrays( 1, &vao );
    glDeleteProgram( program );

    delete camera;
    camera = NULL;

    glBindBuffer( GL_ARRAY_BUFFER, 0 );
    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );

    std::for_each(  meshes.begin(), meshes.end(),
                    []( objMesh_t& mesh ) -> void
                    {
                        glDeleteBuffers( NUM_BUFS_PER_MESH, mesh.vbos );

                        Mem_Free( mesh.vertices );
                        Mem_Free( mesh.indices );

                        mesh.vertices = NULL;
                        mesh.indices  = NULL;
                    } );

    camPtr = NULL;
}

void TLighting::InitLight( void )
{
    light.program = []( void ) -> GLuint
    {
        GLuint shaders[] =
        {
            CompileShader( "src/tests/light/baseVertex.vert", GL_VERTEX_SHADER ),
            CompileShader( "src/tests/light/singleColor.frag", GL_FRAGMENT_SHADER )
        };

        return LinkProgram( shaders, 2 );
    }();

    light.color = glm::vec4( 1.0f );
    light.radius = 10.0f;
    light.ambient = glm::vec4( 1.0f, 1.0f, 1.0f, 1.0f );
    light.intensity = glm::vec4( 0.5f, 0.5f, 0.5f, 1.0f );
    light.worldPos = glm::vec3( 0.0f, 0.0f, -3.0f );

    light.modelScale = 0.2f;

    GLfloat s = 0.3f;

    GLfloat lightCubeVertices[] =
    {
        -s, -s,  s, 1.0f,
        s, -s,  s, 1.0f,
        -s,  s,  s, 1.0f,
        s,  s,  s, 1.0f,
        -s, -s, -s, 1.0f,
        s, -s, -s, 1.0f,
        -s,  s, -s, 1.0f,
        s,  s, -s, 1.0f
    };

    GLuint lightCubeIndices[] =
    {
        // front
        0, 1, 2,
        2, 3, 0,
        // top
        3, 2, 6,
        6, 7, 3,
        // back
        7, 6, 5,
        5, 4, 7,
        // bottom
        4, 5, 1,
        1, 0, 4,
        // left
        4, 0, 3,
        3, 7, 4,
        // right
        1, 5, 6,
        6, 2, 1,
    };

    light.modelIndexCount = UNSIGNED_LEN( lightCubeIndices );

    glGenVertexArrays( 1, &light.vao );
    glGenBuffers( 2, light.vbos );

    glUseProgram( light.program );
    glBindVertexArray( light.vao );
    glBindBuffer( GL_ARRAY_BUFFER, light.vbos[ 0 ] );
    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, light.vbos[ 1 ] );

    glBufferData( GL_ARRAY_BUFFER, sizeof( lightCubeVertices ), lightCubeVertices, GL_STATIC_DRAW );
    glBufferData( GL_ELEMENT_ARRAY_BUFFER, sizeof( lightCubeIndices ), lightCubeIndices, GL_STATIC_DRAW );

    GLuint posAttrib = glGetAttribLocation( light.program, "position" );

    glEnableVertexAttribArray( posAttrib );
    glVertexAttribPointer( posAttrib, 4, GL_FLOAT, GL_FALSE, sizeof( GLfloat ), ( void* )0 );

    glUniform4fv( glGetUniformLocation( light.program, "color" ), 1, glm::value_ptr( light.color ) );
    glUniformMatrix4fv( glGetUniformLocation( light.program, "cameraToClip" ), 1, GL_FALSE, glm::value_ptr( camera->ViewData().clipTransform ) );

    glBindVertexArray( 0 );
    glBindBuffer( GL_ARRAY_BUFFER, 0 );
    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );
    glUseProgram( 0 );
}

glm::vec4 TLighting::CompLightPos( void ) const
{
    glm::vec4 retPos( light.worldPos, 1.0f );

    retPos.x *= light.radius;
    retPos.z *= light.radius;

    return retPos;
}

void TLighting::DrawLight( void ) const
{
    glm::mat4 trans = glm::translate( glm::mat4( 1.0f ), light.worldPos - glm::vec3( 0.0f, 0.0f, 10.0f ) );
    //trans *= glm::scale( glm::mat4( 1.0f ), glm::vec3( 1.0f ) );

    ApplyModelToCameraTransform( light.program, trans, true );

    glBindVertexArray( light.vao);
    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, light.vbos[ 1 ] );
    glDrawElements( GL_TRIANGLES, light.modelIndexCount, GL_UNSIGNED_INT, ( void* ) 0 );
    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );
    glBindVertexArray( 0 );
    glUseProgram( 0 );
}

void TLighting::ApplyModelToCameraTransform( GLuint program, const glm::mat4& model, bool bindProgram ) const
{
    if ( bindProgram )
        glUseProgram( program );

    glUniformMatrix4fv( glGetUniformLocation( program, "modelToCamera" ), 1, GL_FALSE, glm::value_ptr( camera->ViewData().transform * model ) );
}

bool TLighting::Load( void )
{
    if ( !Test::Load( "Lighting Demo" ) )
        return false;

    InitLight();

    glClearColor( 0.0f, 0.3f, 0.3f, 1.0f );

    program = []( void ) -> GLuint
    {
        GLuint shaders[] =
        {
            CompileShader( "src/tests/light/diffuseVertex_MS.vert", GL_VERTEX_SHADER ),
            CompileShader( "src/tests/light/diffusePerFragment_MS.frag", GL_FRAGMENT_SHADER )
        };

        return LinkProgram( shaders, 2 );
    }();

    glBindAttribLocation( program, 0, "inPosition" );
    glBindAttribLocation( program, 1, "inColor" );
    glBindAttribLocation( program, 2, "inNormal" );

    glUseProgram( program );

    GLint posAttrib = glGetAttribLocation( program, "inPosition" );
    GLint colorAttrib = glGetAttribLocation( program, "inColor" );
    //GLint normAttrib = glGetAttribLocation( program, "inNormal" );

    glUniformMatrix4fv( glGetUniformLocation( program, "cameraToClip" ), 1, GL_FALSE, glm::value_ptr( camera->ViewData().clipTransform ) );

    {
        std::vector< shape_t > shapes;

        const std::string& result = LoadObj( shapes, "src/tests/light/Brainy.obj", "src/tests/light/" );

        MyPrintf( "LoadObj Output", "%s", result.c_str() );

        float4_t color =
        {
            0.5f,
            0.5f,
            0.5f,
            1.0f
        };

        for ( int i = 0; i < ( int ) shapes.size(); ++i )
        {
            const std::vector< float >& posRef = shapes[ i ].mesh.positions;
            const std::vector< float >& normRef = shapes[ i ].mesh.normals;
            const std::vector< unsigned >& indRef = shapes[ i ].mesh.indices;

            objMesh_t mesh;

            //mesh.localTransform = glm::translate( glm::mat4( 1.0f ), glm::vec3( 0.0f, 0.0f, 1.0f ) ) * glm::scale( glm::mat4( 1.0f ), glm::vec3( 10.0f ) );

            mesh.localTransform = glm::mat4( 1.0f );

            mesh.numVertices = std::min< unsigned >( posRef.size() / 3, normRef.size() / 3 );
            mesh.vertices = ( objVertex_t* ) Mem_Alloc( sizeof( objVertex_t ) * mesh.numVertices );

            mesh.numIndices = indRef.size();
            mesh.indices = ( GLuint* ) Mem_Alloc( sizeof( GLuint ) * mesh.numIndices );

            int j, k;

            int numFloats = mesh.numVertices * 3;

            for ( j = 0, k = 0; k < numFloats; ++j, k += 3 )
            {
                mesh.vertices[ j ].position.x = posRef[ k     ];
                mesh.vertices[ j ].position.y = posRef[ k + 1 ];
                mesh.vertices[ j ].position.z = posRef[ k + 2 ];

                mesh.vertices[ j ].normal.x = normRef[ k     ];
                mesh.vertices[ j ].normal.y = normRef[ k + 1 ];
                mesh.vertices[ j ].normal.z = normRef[ k + 2 ];

                memcpy( &mesh.vertices[ j ].color, &color, sizeof( float4_t ) );
            }

            for ( j = 0; j < ( int ) indRef.size(); ++j )
                mesh.indices[ j ] = indRef[ j ];

            glGenVertexArrays( 1, &mesh.vao );
            glGenBuffers( NUM_BUFS_PER_MESH, mesh.vbos );

            glBindVertexArray( mesh.vao );

            glBindBuffer( GL_ARRAY_BUFFER, mesh.vbos[ 0 ] );
            glBufferData( GL_ARRAY_BUFFER, sizeof( objVertex_t ) * mesh.numVertices, mesh.vertices, GL_STATIC_DRAW );

            glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, mesh.vbos[ 1 ] );
            glBufferData( GL_ELEMENT_ARRAY_BUFFER, sizeof ( GLuint ) * mesh.numIndices, mesh.indices, GL_STATIC_DRAW );

            glEnableVertexAttribArray( posAttrib );
            glEnableVertexAttribArray( colorAttrib );
            glEnableVertexAttribArray( 2 );

            glVertexAttribPointer( posAttrib, 3, GL_FLOAT, GL_FALSE, sizeof( objVertex_t ), BUFFER_OFFSET( objVertex_t, objVertex_t::position ) );
            glVertexAttribPointer( colorAttrib, 4, GL_FLOAT, GL_FALSE, sizeof( objVertex_t ), BUFFER_OFFSET( objVertex_t, objVertex_t::color ) );
            glVertexAttribPointer( 2, 3, GL_FLOAT, GL_FALSE, sizeof( objVertex_t ), BUFFER_OFFSET( objVertex_t, objVertex_t::normal ) );

            ExitOnGLError( "Loop" );

            glBindVertexArray( 0 );

            meshes.push_back( mesh );
        }
    }

    glUseProgram( 0 );

    return true;
}

void TLighting::Run( void )
{
    camera->Update();

    //DrawLight();

    glUseProgram( program );

    const glm::mat4& modelToCam = camera->ViewData().transform;

    const glm::vec4& lightWorldSpace = CompLightPos();
    const glm::vec4& lightViewSpace = modelToCam * lightWorldSpace;

    const glm::mat4& invCamTransform = glm::inverse( modelToCam );

    glUniform4fv( glGetUniformLocation( program, "lightIntensity" ), 1, glm::value_ptr( light.intensity ) );
    glUniform4fv( glGetUniformLocation( program, "ambientIntensity" ), 1, glm::value_ptr( light.ambient ) );

    for ( int i = 0; i < ( int ) meshes.size(); ++i )
    {
        glBindVertexArray( meshes[ i ].vao );

        ApplyModelToCameraTransform( program, meshes[ i ].localTransform, false );

        const glm::vec4& lightModelSpace = invCamTransform * lightViewSpace;

        glUniform3fv( glGetUniformLocation( program, "modelLightPos" ), 1, glm::value_ptr( lightModelSpace ) );
        glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, meshes[ i ].vbos[ 1 ] );

        glDrawElements( GL_TRIANGLES, meshes[ i ].numIndices, GL_UNSIGNED_INT, ( void* )0 );
    }

    glBindVertexArray( 0 );
    glUseProgram( 0 );
}
