#version 450

smooth in vec4 frag_Color;
smooth in vec2 frag_Tex;
smooth in vec2 frag_Lightmap;

uniform int fragTexIndex;
uniform int fragLightmapIndex;

uniform vec4 fragBias;

uniform sampler2DArray fragSampler;
uniform sampler2DArray fragLightmapSampler;
const float gamma = 1.0 / 2.6;

out vec4 fragment;

void main()
{
	vec4 col = frag_Color;
	
	col *= texture( fragSampler, vec3( frag_Tex * fragBias.xy, float( fragTexIndex ) ) );
	col *= texture( fragLightmapSampler, vec3( frag_Lightmap * fragBias.zw, float( fragLightmapIndex ) ) );

	col.r = pow( col.r, gamma );
	col.g = pow( col.g, gamma );
	col.b = pow( col.b, gamma );

    fragment = col;
}
