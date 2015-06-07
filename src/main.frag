#version 420

smooth in vec4 frag_Color;
smooth in vec2 frag_Tex;
smooth in vec2 frag_Lightmap;

uniform sampler2D fragTexSampler;

uniform sampler2D fragLightmapSampler;

uniform vec4 fragAmbient = vec4( 0.5, 0.5, 0.5, 1.0 );

const float gamma = 1.0 / 2.2;

out vec4 fragment;

void main()
{
    vec4 image = texture( fragTexSampler, frag_Tex );
    vec4 lightmap = texture( fragLightmapSampler, frag_Lightmap );

    vec4 col = frag_Color * image * lightmap;

	col.r = pow( col.r, gamma );
	col.g = pow( col.g, gamma );
	col.b = pow( col.b, gamma );

    fragment = col;
}
