#version 420

in vec3 position;
in vec4 color;
in vec2 tex0;

uniform mat4 modelView;
uniform mat4 projection;

out vec4 frag_Color;
out vec4 frag_Tex;

void main(void)
{
    gl_Position = projection * modelView * vec4( position, 1.0 );

    frag_Color = color;
    frag_Tex = tex0;
}
