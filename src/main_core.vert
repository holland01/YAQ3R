#version 330 core

in vec3 position;
in vec4 color;
in vec2 tex0;
in vec2 lightmap;

uniform mat4 modelToView;
uniform mat4 viewToClip;

out vec4 frag_Color;
out vec2 frag_Tex;
out vec2 frag_Lightmap;

void main()
{
    gl_Position = viewToClip * modelToView * vec4( position, 1.0 );

    frag_Color = color;
    frag_Tex = tex0;
    frag_Lightmap = lightmap;
}
