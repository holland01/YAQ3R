#version 130

in vec3 position;
in vec2 uv;
in vec3 normal;

out vec2 frag_UV;
out vec3 frag_Normal;

vec3 normalToEye( in vec3 normal );
vec4 vertexToClip( in vec3 position );

void main(void)
{
    gl_Position = vertexToClip( position );

    frag_UV = uv;
    frag_Normal = normal;
}
