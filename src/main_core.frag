smooth in vec4 frag_Color;
smooth in vec2 frag_Tex;
smooth in vec2 frag_Lightmap;

uniform sampler2D mainImageSampler;
uniform vec2 mainImageImageScaleRatio;
uniform vec4 mainImageImageTransform;

uniform sampler2D lightmapSampler;
uniform vec2 lightmapImageScaleRatio;
uniform vec4 lightmapImageTransform;

const float gamma = 1.0 / 2.2;

out vec4 fragment;

void main()
{
	vec4 image, lightmap;

	vec2 texCoords = mod( frag_Tex, vec2( 0.99 ) ) * mainImageImageScaleRatio * mainImageImageTransform.zw + mainImageImageTransform.xy;
	image = texture( mainImageSampler, texCoords );

	texCoords = mod( frag_Lightmap, vec2( 0.99 ) ) * lightmapImageScaleRatio * lightmapImageTransform.zw + lightmapImageTransform.xy;
	lightmap = texture( lightmapSampler, texCoords );

	vec4 col = image * lightmap * frag_Color;
	col.r = pow( col.r, gamma );
	col.g = pow( col.g, gamma );
	col.b = pow( col.b, gamma );

	fragment = col;
}
