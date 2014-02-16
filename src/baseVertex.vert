#version 420

in vec3 position;

vec4 vertexToClip( in vec3 position );

void main(void)
{
    gl_Position = vertexToClip( position );
}
