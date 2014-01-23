#version 330

uniform vec4 color0;
uniform vec4 color1;

out vec4 out_Color;

void main(void)
{
    out_Color = vec4( 1.0f, 1.0f, 1.0f, mix( color0.a, color1.a, 1.0 ) );
}
