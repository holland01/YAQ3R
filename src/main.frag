#version 420

smooth in vec4 frag_Color;
//smooth in vec2 frag_Tex;

//uniform sampler2D texSampler;

out vec4 fragment;

void main()
{
    //fragment = texture(texSampler, frag_Tex);
    fragment = frag_Color;
}
