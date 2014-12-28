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

out vec4 fragment;

void main()
{
	switch ( fragWriteMode )
	{
	case FRAGWRITE_TEX:
		fragment = texture(fragTexSampler, frag_Tex) * texture(fragLightmapSampler, frag_Lightmap);
		break;
	case FRAGWRITE_TEX_COLOR:
		fragment = texture(fragTexSampler, frag_Tex) * vec4( texture(fragLightmapSampler, frag_Lightmap).xyz, 1.0 ) * frag_Color * fragAmbient;
		break;
	case FRAGWRITE_COLOR:
		fragment = frag_Color;
		break;
    }
}
