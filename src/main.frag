#version 420

smooth in vec4 frag_Color;
smooth in vec2 frag_Tex;
smooth in vec2 frag_Lightmap;

uniform sampler2D fragTexSampler;

uniform sampler2D fragLightmapSampler;

uniform vec4 fragAmbient = vec4( 0.3, 0.5, 0.7, 1.0 );

const float gamma = 2.2;

const vec4 gammaDecode = vec4( gamma );
const vec4 gammaEncode = vec4( 1 / gamma );

out vec4 fragment;

void main()
{
	vec4 col;

	vec4 image = texture( fragTexSampler, frag_Tex );
	vec4 lightmap = texture( fragLightmapSampler, frag_Lightmap );

	col = image * lightmap * frag_Color * fragAmbient;

	fragment = pow( col, gammaEncode );
}
