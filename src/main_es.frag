#version 100

precision mediump float;

varying vec4 frag_Color;
varying vec2 frag_Tex;
varying vec2 frag_Lightmap;

struct ImageParams
{
    sampler2D sampler0;
    vec2 imageScaleRatio;
    vec4 imageTransform;
    bool active;
};

uniform ImageParams mainImage;
uniform ImageParams lightmap;

const float gamma = 1.0 / 3.0;

vec4 FetchTexture( in ImageParams params, vec2 originalCoords )
{
    if ( params.active )
    {
        vec2 texCoords = mod( originalCoords, 0.99 ) * params.imageScaleRatio * params.imageTransform.zw + params.imageTransform.xy;
        return texture2D( params.sampler0, texCoords );
    }

    return vec4( 1.0 );
}

void main()
{
    vec4 col = frag_Color;

    col *= FetchTexture( mainImage, frag_Tex );
    col *= FetchTexture( lightmap, frag_Lightmap );

    col.r = pow( col.r, gamma );
    col.g = pow( col.g, gamma );
    col.b = pow( col.b, gamma );

    gl_FragColor = col;
}
