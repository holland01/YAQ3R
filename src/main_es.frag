#version 100

precision mediump float;
precision mediump sampler2D;

varying vec4 frag_Color;
varying vec2 frag_Tex;
varying vec2 frag_Lightmap;

uniform sampler2D mainImageSampler;
uniform vec2 mainImageImageScaleRatio;
uniform vec4 mainImageImageTransform;

uniform sampler2D lightmapSampler;
uniform vec2 lightmapImageScaleRatio;
uniform vec4 lightmapImageTransform;

const float gamma = 1.0 / 3.0;

void main()
{
	vec4 image, lightmap;

	vec2 texCoords = mod( frag_Tex, vec2( 0.99 ) ) * mainImageImageScaleRatio * mainImageImageTransform.zw + mainImageImageTransform.xy;
	image = texture2D( mainImageSampler, texCoords );
	if ( image.xyz == vec3( 0.0 ) )
		image = vec4( 1.0 );

	texCoords = mod( frag_Lightmap, vec2( 0.99 ) ) * lightmapImageScaleRatio * lightmapImageTransform.zw + lightmapImageTransform.xy;
	lightmap = texture2D( lightmapSampler, texCoords );
	if ( lightmap.xyz == vec3( 0.0 ) )
		lightmap = vec4( 1.0 );

	vec4 col = image * lightmap * frag_Color;
	col.r = pow( col.r, gamma );
	col.g = pow( col.g, gamma );
	col.b = pow( col.b, gamma );

	gl_FragColor = col;
}
