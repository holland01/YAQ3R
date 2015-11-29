#version 450

smooth in vec4 frag_Color;
smooth in vec2 frag_Tex;
smooth in vec2 frag_Lightmap;

uniform sampler2DArray fragSampler;
uniform sampler2DArray fragLightmapSampler;
uniform vec3 fragBiases[ 2 ];

const float gamma = 1.0 / 2.2;

out vec4 fragment;

void main()
{
	vec4 col = frag_Color;

	if ( fragBiases[ 0 ].z != -1 )
	{
		vec3 texCoords = vec3( frag_Tex * fragBiases[ 0 ].xy, fragBiases[ 0 ].z );
		col *= texture( fragSampler, texCoords );
	}

	if ( fragBiases[ 1 ].z != -1 )
	{
		vec3 lmCoords = vec3( frag_Lightmap * fragBiases[ 1 ].xy, fragBiases[ 1 ].z );
		col *= texture( fragLightmapSampler, lmCoords );
	}
	
	col.r = pow( col.r, gamma );
	col.g = pow( col.g, gamma );
	col.b = pow( col.b, gamma );

    fragment = col;
}
