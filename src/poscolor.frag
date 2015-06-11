#version 420

smooth in vec4 frag_Color;
smooth in vec2 frag_Lightmap;

uniform sampler2D fragLightmapSampler;

layout( location = 0 ) out vec4 fragment;

const float gamma = 1.0 / 2.4;

void main( void )
{
	vec4 color = frag_Color * texture( fragLightmapSampler, frag_Lightmap );

	color.r = pow( color.r, gamma );
	color.g = pow( color.g, gamma );
	color.b = pow( color.b, gamma );

	fragment = color;
}