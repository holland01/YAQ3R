#version 420

const int FRAGWRITE_TEX = 0;
const int FRAGWRITE_TEX_COLOR = 1;
const int FRAGWRITE_COLOR = 2;

smooth in vec4 frag_Color;
smooth in vec2 frag_Tex;

uniform sampler2D fragTexSampler;
uniform int fragWriteMode;

out vec4 fragment;


void main()
{
	switch ( fragWriteMode )
	{
	case FRAGWRITE_TEX:
		fragment = texture(fragTexSampler, frag_Tex);
		break;
	case FRAGWRITE_TEX_COLOR:
		fragment = texture(fragTexSampler, frag_Tex) * frag_Color;
		break;
	case FRAGWRITE_COLOR:
		fragment = frag_Color;
		break;
    }
}
