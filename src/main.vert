#version 130

in vec3 position;
in vec4 color;

uniform mat4 modelview;
uniform mat4 projection;

out vec4 frag_Color;

void main(void)
{
    gl_Position = projection * modelview * vec4( position, 1.0 );

    frag_Color = color;
}
