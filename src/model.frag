#version 450

smooth in vec4 frag_Color;
smooth in vec2 frag_Tex;
smooth in vec2 frag_Lightmap;
smooth in vec3 frag_Normal;

uniform vec3 fragDirToLight;
uniform vec3 fragDirectional;
uniform vec3 fragAmbient;

uniform sampler2D fragTexSampler;
uniform sampler2D fragLightmapSampler;

const float gamma = 1.0 / 2.4;

out vec4 fragment;

void main()
{
	float diffuse = clamp( dot( fragDirToLight, normalize( frag_Normal ) ), 0.0, 1.0 );
		
    vec4 image = texture( fragTexSampler, frag_Tex );
    vec4 lightmap = texture( fragLightmapSampler, frag_Lightmap );

	vec4 amblight4 = vec4( fragAmbient, 1.0 );
	vec4 dirlight4 = vec4( fragDirectional, 1.0 );
	 
    vec4 col = image * lightmap * ( frag_Color + diffuse + amblight4 + dirlight4 );

	col.r = pow( col.r, gamma );
	col.g = pow( col.g, gamma );
	col.b = pow( col.b, gamma );

    fragment = col;
}
