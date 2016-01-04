//#version 100

attribute vec3 position;
attribute vec4 color;
attribute vec2 tex0;
attribute vec2 lightmap;

uniform mat4 modelToView;
uniform mat4 viewToClip;

varying vec4 frag_Color;
varying vec2 frag_Tex;
varying vec2 frag_Lightmap;

void main()
{
    gl_Position = viewToClip * modelToView * vec4( position, 1.0 );

    frag_Color = color;
    frag_Tex = tex0;
    frag_Lightmap = lightmap;
}
