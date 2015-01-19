#version 420

const int FRAGWRITE_TEX = 0;
const int FRAGWRITE_TEX_COLOR = 1;
const int FRAGWRITE_COLOR = 2;

smooth in vec4 frag_Color;
smooth in vec2 frag_Tex;
smooth in vec2 frag_Lightmap;

uniform sampler2D fragTexSampler;

uniform sampler2D fragLightmapSampler;

uniform int fragWriteMode;

uniform vec4 fragAmbient = vec4( 1.0 );

const vec4 gamma = vec4( 1 / 2.2 );

out vec4 fragment;

void main()
{
	vec4 col;

	switch ( fragWriteMode )
	{
	case FRAGWRITE_TEX:
		col = texture(fragTexSampler, frag_Tex) * texture(fragLightmapSampler, frag_Lightmap);
		break;
	case FRAGWRITE_TEX_COLOR:
		col = vec4( texture(fragTexSampler, frag_Tex).rgb, 1.0 ) * texture(fragLightmapSampler, frag_Lightmap) * vec4( frag_Color.rgb, 1.0 ) * fragAmbient;
		break;
	case FRAGWRITE_COLOR:
		col = frag_Color * fragAmbient;
		break;
    }

	fragment = pow( col, gamma );
}
