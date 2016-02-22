#version 100

attribute vec3 position;

uniform mat4 modelToView;
uniform mat4 viewToClip;

void main(void)
{
	gl_Position = viewToClip * modelToView * vec4(position, 1.0);
}
