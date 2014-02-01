#version 130

//uniform vec4 color0;
//uniform vec4 color1;
in vec4 frag_Color;
out vec4 out_Color;

uniform vec4 color0;

void main(void)
{
    out_Color = color0;
}
