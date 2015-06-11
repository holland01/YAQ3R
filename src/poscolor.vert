#version 420

layout( location = 0 ) in vec3 position;
layout( location = 1 ) in vec4 color;
layout( location = 2 ) in vec2 lightmap;

layout( std140 ) uniform Transforms 
{
	mat4 modelToView;
	mat4 viewToClip;
};

smooth out vec4 frag_Color;
smooth out vec2 frag_Lightmap;

void main(void)
{
	gl_Position = viewToClip * modelToView * vec4( position, 1.0 );

	frag_Color = color;
	frag_Lightmap = lightmap;
}