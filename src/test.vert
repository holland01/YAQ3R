#version 330

layout(location = 0) in vec3 position;

uniform mat4 modelView;
uniform mat4 projection;

void main(void)
{
    gl_Position = projection * modelView * vec4( position, 1.0f );
}
