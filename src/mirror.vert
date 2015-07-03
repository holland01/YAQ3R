#version 450

layout( location = 0 ) in vec3 position;
layout( location = 1 ) in vec4 color;

uniform mat4 modelToView;
uniform mat4 viewToClip;

void main(void) 
{
	gl_Position = viewToClip * modelToView * vec4( position, 1.0 );
}
