#version 420

in vec3 position;
in vec4 color;
in vec2 tex0;
in vec2 lightmap;

layout( std140 ) uniform Transforms
{
	mat4 viewToClip;
	mat4 modelToView;
};

smooth out vec4 frag_Color;
smooth out vec2 frag_Tex;
smooth out vec2 frag_Lightmap;

const vec4 gamma = vec4( 2.2 );

void main()
{
    gl_Position = viewToClip * modelToView * vec4( position, 1.0 );

    frag_Color = color;

    frag_Tex = tex0;
    frag_Lightmap = lightmap;
}
