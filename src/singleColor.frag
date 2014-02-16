#version 420

uniform vec4 fragmentColor;

out vec4 out_Color;

void main(void)
{
    out_Color = fragmentColor;
}
