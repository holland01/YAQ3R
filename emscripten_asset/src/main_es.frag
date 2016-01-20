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
    vec4 col = frag_Color;

    vec2 texCoords = mod( frag_Tex, vec2( 0.99 ) ) * mainImageImageScaleRatio * mainImageImageTransform.zw + mainImageImageTransform.xy;
    col *= texture2D( mainImageSampler, texCoords );

    texCoords = mod( frag_Lightmap, vec2( 0.99 ) ) * lightmapImageScaleRatio * lightmapImageTransform.zw + lightmapImageTransform.xy;
    col *= texture2D( lightmapSampler, texCoords );

    col.r = pow( col.r, gamma );
    col.g = pow( col.g, gamma );
    col.b = pow( col.b, gamma );

    gl_FragColor = col;
}
