#version 420

in vec2 frag_UV;

uniform sampler2D sampler;

out vec4 out_Color;

void main(void)
{
    out_Color = texture( sampler, frag_UV );
}
