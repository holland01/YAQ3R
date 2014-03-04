 #include "tlighting.h"
#include "key_mover.h"
#include "../log.h"
#include "../shader.h"

namespace
{
    const int LIGHTCUBE_VERTS_LEN = 24 * 3 * 2;

    GLfloat s = 0.01f;

    GLfloat lightCubeVertices[ LIGHTCUBE_VERTS_LEN ] =
    {
        -s,-s,-s, // triangle 1 : begin
        -s,-s, s,
        -s, s, s, // triangle 1 : end
        s, s,-s, // triangle 2 : begin
        -s,-s,-s,
        -s, s,-s, // triangle 2 : end
        s,-s, s,
        -s,-s,-s,
        s,-s,-s,
        s, s,-s,
        s,-s,-s,
        -s,-s,-s,
        -s,-s,-s,
        -s, s, s,
        -s, s,-s,
        s,-s, s,
        -s,-s, s,
        -s,-s,-s,
        -s, s, s,
        -s,-s, s,
        s,-s, s,
        s, s, s,
        s,-s,-s,
        s, s,-s,
        s,-s,-s,
        s, s, s,
        s,-s, s,
        s, s, s,
        s, s,-s,
        -s, s,-s,
        s, s, s,
        -s, s,-s,
        -s, s, s,
        s, s, s,
        -s, s, s,
        s,-s, s
    };

    void GenFloatArrayNormals( float* arrayBuffer, int start, int end )
    {
        const int NORMALS_START = start; //LIGHTCUBE_VERTS_LEN / 2;
        const int NORMALS_END   = end;//LIGHTCUBE_VERTS_LEN;

        int vertex = 0;

        for ( int i = NORMALS_START; i < NORMALS_END; i += 3 )
        {
            glm::vec3 normal( 0.0f ), adjCrossSum;

            // Begin at our base vertex
            adjCrossSum.x = arrayBuffer[ vertex     ];
            adjCrossSum.y = arrayBuffer[ vertex + 1 ];
            adjCrossSum.z = arrayBuffer[ vertex + 2 ];

            // Iterate through three adjacent vertices and
            // Sum them up each to compute the normalized average
            int sumStart, sumEnd;

            sumStart = vertex;
            sumEnd   = vertex + 9;

            const int BOUNDS = vertex == 0 ? NORMALS_START : vertex; //NORMALS_START + vertex;

            int interm0 = BOUNDS;
            int interm1 = BOUNDS - 3;

            int interm0Add = 3;
            int interm1Add = 3; // Delay addition until after second iteration

            for ( int tri = sumStart; tri < sumEnd; tri += 3 )
            {
                glm::vec3 currentVert, lastVert;

                int currentBase = interm0;

                currentVert.x = arrayBuffer[ currentBase     ];
                currentVert.y = arrayBuffer[ currentBase + 1 ];
                currentVert.z = arrayBuffer[ currentBase + 2 ];

                int lastBase = interm1;

                lastVert.x = arrayBuffer[ lastBase     ];
                lastVert.y = arrayBuffer[ lastBase + 1 ];
                lastVert.z = arrayBuffer[ lastBase + 2 ];

                adjCrossSum += glm::cross( currentVert, lastVert );

                // FIXME: lastBase is off by -3 in last iteration of first normal: when currentBase = 6, lastBase = 0.
                // lastBase should be == 3
                if ( BOUNDS == NORMALS_START )
                {
                    // First iteration
                    if ( interm0 == BOUNDS )
                    {
                        interm0 = vertex - 3;
                    }
                    // Second iteration
                    else if ( interm1 == BOUNDS )
                    {
                        interm1 = vertex - 3;
                    }
                }

                interm0 += interm0Add;
                interm1 += interm1Add;
            }

            normal = glm::normalize( adjCrossSum );

            arrayBuffer[ i     ] = normal.x;
            arrayBuffer[ i + 1 ] = normal.y;
            arrayBuffer[ i + 2 ] = normal.z;

            vertex += 3;
        }
    }
}

void FreePointLight( pointLight_t* light )
{
    if ( light )
    {
        glDeleteVertexArrays( 1, &light->vao );
        glDeleteBuffers( 1, &light->vbo );
        glDeleteProgram( light->program );

        if ( light->mover )
            delete light->mover;
    }
}

TLighting::TLighting( void )
    : Test( 1366, 768 ),
      camera( new Camera ),
      program( 0 )
{
    camPtr = camera;
}

TLighting::~TLighting( void )
{
    FreePointLight( &light );

    glDeleteVertexArrays( 1, &vao );
    glDeleteProgram( program );

    delete camera;
    camera = NULL;

    glBindBuffer( GL_ARRAY_BUFFER, 0 );
    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );

    for ( int i = 0; i < ( int ) meshes.size(); ++i )
    {
        objMesh_t& m = meshes[ i ];

        glDeleteBuffers( NUM_BUFS_PER_MESH, m.vbos );

        Mem_Free( m.vertices );
        Mem_Free( m.indices );

        m.vertices = NULL;
        m.indices  = NULL;
    }

    camPtr = NULL;
}

void TLighting::InitLight( void )
{
    GenFloatArrayNormals(
                lightCubeVertices,
                LIGHTCUBE_VERTS_LEN / 2, // start of normals
                LIGHTCUBE_VERTS_LEN      // end of normals
            );

    light.program = []( void ) -> GLuint
    {
        GLuint shaders[] =
        {
            CompileShader( "src/tests/light/pointLightModel.vert", GL_VERTEX_SHADER ),
            CompileShader( "src/tests/light/pointLightModel.frag", GL_FRAGMENT_SHADER )
        };

        return LinkProgram( shaders, 2 );
    }();


    light.radius = 5.0f;
    light.specularShininess = 20.0f;
    light.specularStrength = 10.0f;

    light.ambient = glm::vec4( 1.0f, 0.0f, 0.0f, 1.0f );
    light.diffuse = light.ambient;
    light.intensity = glm::vec4( 1.0f, 1.0f, 1.0f, 1.0f );
    light.worldPos = glm::vec3( 0.0f, -0.1f, 0.0f );

    light.mover = new KeyMover( light.worldPos, 0.01f );

    light.modelNumVertices = UNSIGNED_LEN( lightCubeVertices ) / 2 / 3;

    glGenVertexArrays( 1, &light.vao );
    glGenBuffers( 1, &light.vbo );

    glUseProgram( light.program );
    glBindVertexArray( light.vao );

    glBindBuffer( GL_ARRAY_BUFFER, light.vbo );
    glBufferData( GL_ARRAY_BUFFER, sizeof( lightCubeVertices ), lightCubeVertices, GL_STATIC_DRAW );

    GLuint posAttrib = glGetAttribLocation( light.program, "inPosition" );
    //GLuint normalAttrib = glGetAttribLocation( light.program, "inNormal" );

    glEnableVertexAttribArray( posAttrib );
    glVertexAttribPointer( posAttrib, 3, GL_FLOAT, GL_FALSE, 0, ( void* )0 );
    //glVertexAttribPointer( normalAttrib, 3, GL_FLOAT, GL_FALSE, 0, ( void* )( sizeof( float ) * ( LIGHTCUBE_VERTS_LEN / 2 ) ) );

    glUniform4fv( glGetUniformLocation( light.program, "objectColor" ), 1, glm::value_ptr( light.diffuse ) );
    glUniformMatrix4fv( glGetUniformLocation( light.program, "cameraToClip" ), 1, GL_FALSE, glm::value_ptr( camera->ViewData().clipTransform ) );

    glBindVertexArray( 0 );
    glBindBuffer( GL_ARRAY_BUFFER, 0 );
    glUseProgram( 0 );
}

glm::vec4 TLighting::CompLightPos( void ) const
{
    glm::vec4 retPos( light.worldPos, 1.0f );

    retPos.x *= light.radius;
    retPos.z *= light.radius;

    return retPos;
}

void TLighting::DrawLight( const glm::vec4& lightWorldSpace ) const
{
    glm::mat4 trans = glm::translate( glm::mat4( 1.0f ), light.worldPos );

    glUseProgram( light.program );

    ApplyModelToCameraTransform( light.program, trans );

    glBindVertexArray( light.vao );
        glBindBuffer( GL_ARRAY_BUFFER, light.vbo );
        glDrawArrays( GL_TRIANGLE_FAN, 0, light.modelNumVertices );
    glBindVertexArray( 0 );


    glUseProgram( 0 );
}

void TLighting::DrawModel( const glm::vec4& lightWorldSpace ) const
{
    glUseProgram( program );

    const glm::mat4& modelToCam = camera->ViewData().transform;

    const glm::vec4& lightViewSpace = modelToCam * lightWorldSpace;

    const glm::mat4& invCamTransform = glm::inverse( modelToCam );
    const glm::vec4& lightModelSpace = invCamTransform * lightViewSpace;

    const glm::vec4& eyeWorldSpace = modelToCam * camera->ViewData().orientation * ( glm::vec4( camera->ViewData().origin, 1.0f ) - modelToCam[ 3 ] );
    //const glm::vec4& eyeWorldSpace = glm::vec4( camera->ViewData().origin, 1.0f );

    const glm::vec4& eyeViewSpace = modelToCam * eyeWorldSpace;
    const glm::vec4& eyeModelSpace = invCamTransform * eyeViewSpace;

    const glm::vec3& halfVector = glm::vec3( eyeWorldSpace - lightWorldSpace );
    //const glm::vec3& halfVector = glm::vec3( eyeModelSpace - lightModelSpace );

    glUniform1f( glGetUniformLocation( program, "strength" ), light.specularStrength );
    glUniform1f( glGetUniformLocation( program, "shininess" ), light.specularShininess );

    glUniform3fv( glGetUniformLocation( program, "halfVector" ), 1, glm::value_ptr( halfVector ) );

    glUniform4fv( glGetUniformLocation( program, "lightIntensity" ), 1, glm::value_ptr( light.intensity ) );
    glUniform4fv( glGetUniformLocation( program, "ambientIntensity" ), 1, glm::value_ptr( light.ambient ) );


    for ( int i = 0; i < ( int ) meshes.size(); ++i )
    {
        glBindVertexArray( meshes[ i ].vao );

        ApplyModelToCameraTransform( program, meshes[ i ].localTransform );

        glm::mat3 normalTransform( meshes[ i ].localTransform );

        glUniformMatrix3fv( glGetUniformLocation( program, "normalTransform" ), 1, GL_FALSE, glm::value_ptr( normalTransform ) );

        glUniform3fv( glGetUniformLocation( program, "modelLightPos" ), 1, glm::value_ptr( lightModelSpace ) );

        glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, meshes[ i ].vbos[ 1 ] );

        glDrawElements( GL_TRIANGLES, meshes[ i ].numIndices, GL_UNSIGNED_INT, ( void* )0 );
    }

    glBindVertexArray( 0 );
    glUseProgram( 0 );
}

void TLighting::ApplyModelToCameraTransform( GLuint program, const glm::mat4& model ) const
{
    glUniformMatrix4fv( glGetUniformLocation( program, "modelToCamera" ), 1, GL_FALSE, glm::value_ptr( camera->ViewData().transform * model ) );
}

bool TLighting::Load( void )
{
    if ( !Test::Load( "Lighting Demo" ) )
        return false;

    InitLight();

    glClearColor( 0.0f, 0.0f, 0.0f, 1.0f );

    program = []( void ) -> GLuint
    {
        GLuint shaders[] =
        {
            CompileShader( "src/tests/light/diffuseVertex_MS.vert", GL_VERTEX_SHADER ),
            CompileShader( "src/tests/light/diffusePerFragment_MS.frag", GL_FRAGMENT_SHADER )
        };

        return LinkProgram( shaders, 2 );
    }();

    glUseProgram( program );

    glBindAttribLocation( program, 0, "inPosition" );
    glBindAttribLocation( program, 1, "inColor" );
    glBindAttribLocation( program, 2, "inNormal" );

    GLint posAttrib = glGetAttribLocation( program, "inPosition" );
    GLint colorAttrib = glGetAttribLocation( program, "inColor" );
    GLint normAttrib = glGetAttribLocation( program, "inNormal" );

    glUniformMatrix4fv( glGetUniformLocation( program, "cameraToClip" ), 1, GL_FALSE, glm::value_ptr( camera->ViewData().clipTransform ) );

    {
        std::vector< shape_t > shapes;

        const std::string& result = LoadObj( shapes, "src/tests/light/Brainy.obj", "src/tests/light/" );

        MyPrintf( "LoadObj Output", "%s", result.c_str() );

        float4_t color =
        {
            {
                0.8f,
                0.5f,
                0.5f,
                1.0f
            }
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
            //ExitOnGLError( "GenVertexArrays" );

            glBindVertexArray( mesh.vao );
            //ExitOnGLError( "glBindVertexArray" );

            glGenBuffers( NUM_BUFS_PER_MESH, mesh.vbos );
            //ExitOnGLError( "GenBuffers" );

            glBindBuffer( GL_ARRAY_BUFFER, mesh.vbos[ 0 ] );
            //ExitOnGLError( "glBindBuffer GL_ARRAY_BUFFER" );

            glBufferData( GL_ARRAY_BUFFER, sizeof( objVertex_t ) * mesh.numVertices, mesh.vertices, GL_STATIC_DRAW );
            //ExitOnGLError( "glBufferData GL_ARRAY_BUFFER" );

            glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, mesh.vbos[ 1 ] );
            //ExitOnGLError( "glBindBuffer GL_ELEMENT_ARRAY_BUFFER" );

            glBufferData( GL_ELEMENT_ARRAY_BUFFER, sizeof ( GLuint ) * mesh.numIndices, mesh.indices, GL_STATIC_DRAW );
            //ExitOnGLError( "glBufferData GL_ELEMENT_ARRAY_BUFFER" );

            glEnableVertexAttribArray( posAttrib );
            //ExitOnGLError( "glEnableVertexAttribArray 0" );

            glEnableVertexAttribArray( colorAttrib );
            //ExitOnGLError( "glEnableVertexAttribArray 1" );

            glEnableVertexAttribArray( normAttrib );
            //ExitOnGLError( "glEnableVertexAttribArray 2" );

            glVertexAttribPointer( posAttrib, 3, GL_FLOAT, GL_FALSE, sizeof( objVertex_t ), BUFFER_OFFSET( objVertex_t, objVertex_t::position ) );
            //ExitOnGLError( "glEnableVertexAttribArray 0" );

            glVertexAttribPointer( colorAttrib, 4, GL_FLOAT, GL_FALSE, sizeof( objVertex_t ), BUFFER_OFFSET( objVertex_t, objVertex_t::color ) );
            //ExitOnGLError( "glEnableVertexAttribArray 1" );

            glVertexAttribPointer( normAttrib, 3, GL_FLOAT, GL_FALSE, sizeof( objVertex_t ), BUFFER_OFFSET( objVertex_t, objVertex_t::normal ) );
            //ExitOnGLError( "glEnableVertexAttribArray 2" );

            glBindVertexArray( 0 );
            //ExitOnGLError( "glBindVertexArray 0" );

            meshes.push_back( mesh );
        }
    }

    glUseProgram( 0 );

    return true;
}

void TLighting::Run( void )
{
    light.mover->Update();
    camera->Update();

    const glm::vec4& lightWorldSpace = CompLightPos();

    DrawLight( lightWorldSpace );
    DrawModel( lightWorldSpace );
}

void TLighting::OnKeyPress( int key, int scancode, int action, int mods )
{
    Test::OnKeyPress( key, scancode, action, mods );

    switch( action )
    {
        case GLFW_PRESS:
            light.mover->EvalKeyPress( key );
            break;
        case GLFW_RELEASE:
            light.mover->EvalKeyRelease( key );
            break;
    }
}
