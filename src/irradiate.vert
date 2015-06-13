#version 420

layout( location = 0 ) in vec3 position;
layout( location = 1 ) in vec3 normal;

layout( std140 ) uniform Transforms 
{
	mat4 modelToView;
	mat4 viewToClip;
};

smooth out vec3 frag_Normal;
smooth out vec3 frag_Position;

void main(void)
{
	gl_Position = viewToClip * modelToView * vec4( position, 1.0 );
	frag_Position = position;
	frag_Normal = normal;
}