#version 330

uniform vec4 color0;
uniform vec4 color1;

out vec4 out_Color;

void main(void)
{
    out_Color = mix( color0, color1, 0.5 );
}
