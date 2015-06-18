#version 420

smooth in vec4 frag_Color;
smooth in vec2 frag_Tex;
//smooth in vec2 frag_Lightmap;
smooth in vec3 frag_Normal;

uniform sampler2D fragTexSampler;
//uniform sampler2D fragLightmapSampler;
uniform samplerCube fragIrradianceSampler;

//uniform vec3 fragCubeFace = vec3( 1.0, 0.0, 0.0 );

const float gamma = 1.0 / 2.4;

out vec4 fragment;

void main()
{
    vec4 image = texture( fragTexSampler, frag_Tex );
   // vec4 lightmap = texture( fragLightmapSampler, frag_Lightmap );
	vec4 lighting = texture( fragIrradianceSampler, frag_Normal );

    vec4 col = frag_Color * image * lighting;

	col.r = pow( col.r, gamma );
	col.g = pow( col.g, gamma );
	col.b = pow( col.b, gamma );

    fragment = col;
}
