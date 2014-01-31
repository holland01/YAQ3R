#version 130

//uniform vec4 color0;
//uniform vec4 color1;
in vec4 frag_Color;
out vec4 out_Color;

void main(void)
{
    out_Color = frag_Color;
}
