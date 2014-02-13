#version 420

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;

uniform mat4 projection;
uniform mat4 model;

out vec4 frag_Color;

void main(void)
{
    gl_Position = projection * model * vec4( position, 1.0 );
    frag_Color = vec4( color, 1.0 );
}
