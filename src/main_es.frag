//#version 100

precision mediump float;
precision mediump sampler2D;

varying vec4 frag_Color;
varying vec2 frag_Tex;
varying vec2 frag_Lightmap;

struct ImageParams
{
    vec2 imageScaleRatio;
    vec4 imageTransform;
    int active;
};

uniform sampler2D mainImageSampler;
uniform vec2 mainImageImageScaleRatio;
uniform vec4 mainImageImageTransform;
int mainImageActive;

uniform sampler2D lightmapSampler;
uniform vec2 lightmapImageScaleRatio;
uniform vec4 lightmapImageTransform;
int lightmapActive;

const float gamma = 1.0 / 3.0;

void main()
{
    vec4 col = frag_Color;

    if (mainImageActive == 1)
    {
        vec2 texCoords = mod( frag_Tex, vec2( 1.0 ) ) * mainImageImageScaleRatio * mainImageImageTransform.zw + mainImageImageTransform.xy;
        col *= texture2D( mainImageSampler, texCoords );
    }

    if (lightmapActive == 1)
    {
        vec2 texCoords = mod( frag_Lightmap, vec2( 1.0 ) ) * lightmapImageScaleRatio * lightmapImageTransform.zw + lightmapImageTransform.xy;
        col *= texture2D( lightmapSampler, texCoords );
    }

    col.r = pow( col.r, gamma );
    col.g = pow( col.g, gamma );
    col.b = pow( col.b, gamma );

    gl_FragColor = col;
}
