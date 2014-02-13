#version 420

in vec4 frag_Color;
in vec2 frag_Tex;

uniform float deltaTime;
uniform sampler2D sampler;

out vec4 out_Color;

void main(void)
{
    out_Color = mix(texture(sampler, frag_Tex, 0.0, 1.0), frag_Color, deltaTime);
}
