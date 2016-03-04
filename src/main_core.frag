#version 330 core

//smooth in vec4 frag_Color;
smooth in vec2 frag_Tex;
//smooth in vec2 frag_Lightmap;

uniform sampler2D mainImageSampler;
uniform vec2 mainImageImageScaleRatio;
uniform vec4 mainImageImageTransform;

const float gamma = 1.0 / 3.0;

out vec4 fragment;

void main()
{
	vec2 texCoords = mod( frag_Tex, vec2( 0.99 ) ) * mainImageImageScaleRatio * mainImageImageTransform.zw + mainImageImageTransform.xy;
	vec4 image = texture( mainImageSampler, texCoords );
	if ( image.xyz == vec3( 0.0 ) )
		image = vec4( 1.0, 0.0, 0.0, 1.0 );

	vec4 col = image;// * lightmap * frag_Color;
	col.r = pow( col.r, gamma );
	col.g = pow( col.g, gamma );
	col.b = pow( col.b, gamma );

	fragment = col;
}
