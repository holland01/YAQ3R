#version 420

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 uv;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec2 frag_UV;

void main(void)
{
    gl_Position = projection * view * model * vec4( position, 1.0 );

    frag_UV = uv;
}
