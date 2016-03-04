#version 330 core

in vec4 frag_Color;
in vec2 frag_Tex;
in vec2 frag_Lightmap;

uniform sampler2D mainImageSampler;
uniform vec2 mainImageImageScaleRatio;
uniform vec4 mainImageImageTransform;

uniform sampler2D lightmapSampler;
uniform vec2 lightmapImageScaleRatio;
uniform vec4 lightmapImageTransform;

const float gamma = 1.0 / 3.0;

out vec4 fragment;

void main()
{
	vec4 image, lightmap = vec4( 1.0 );

	vec2 texCoords = mod( frag_Tex, vec2( 0.99 ) ) * mainImageImageScaleRatio * mainImageImageTransform.zw + mainImageImageTransform.xy;
	image = texture( mainImageSampler, texCoords );
	if ( image.xyz == vec3( 0.0 ) )
		image = vec4( 1.0 );

/*
	texCoords = mod( frag_Lightmap, vec2( 0.99 ) ) * lightmapImageScaleRatio * lightmapImageTransform.zw + lightmapImageTransform.xy;
	lightmap = texture( lightmapSampler, texCoords ).rrra;
	if ( lightmap.xyz == vec3( 0.0 ) )
		lightmap = vec4( 1.0 );
*/

	vec4 col = image * lightmap * frag_Color;
	col.r = pow( col.r, gamma );
	col.g = pow( col.g, gamma );
	col.b = pow( col.b, gamma );

	fragment = col;
}
