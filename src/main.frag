#version 420

smooth in vec4 frag_Color;
smooth in vec2 frag_Tex;
smooth in vec2 frag_Lightmap;
smooth in vec3 frag_Normal;

// light volume data
uniform vec3 fragAmbient;
uniform vec3 fragDirectional;
uniform vec3 fragDirToLight; 

uniform sampler2D fragTexSampler;
uniform sampler2D fragLightmapSampler;

const float gamma = 1.0 / 3.0;

out vec4 fragment;

void main()
{
	vec3 diffuse = clamp( dot( fragLightDir, normalize( frag_Normal ), 0.0, 1.0 );

    vec4 image = texture( fragTexSampler, frag_Tex );
    vec4 lightmap = texture( fragLightmapSampler, frag_Lightmap );

    vec4 col = frag_Color * ( image * lightmap + fragDirectional + diffuse + fragAmbient );

	col.r = pow( col.r, gamma );
	col.g = pow( col.g, gamma );
	col.b = pow( col.b, gamma );

    fragment = col;
}
