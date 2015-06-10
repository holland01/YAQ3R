#version 420

smooth in vec4 frag_Color;
smooth in vec2 frag_Tex;
smooth in vec2 frag_Lightmap;

uniform sampler2D fragTexSampler;
uniform sampler2D fragLightmapSampler;

const float gamma = 1.0 / 2.4;

layout( location = 0 ) out vec4 fragment;

void main(void)
{
	
}