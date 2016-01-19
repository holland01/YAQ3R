#version 100

attribute vec3 position;

layout( std140 ) uniform Transforms
{
	mat4 viewToClip;
	mat4 modelToView;
};

void main()
{
    gl_Position = viewToClip * modelToView * vec4( position, 1.0 );
}
