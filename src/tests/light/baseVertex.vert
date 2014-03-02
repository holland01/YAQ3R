#version 420

layout( location = 0 ) in vec4 position;

uniform mat4 modelToCamera;
uniform mat4 cameraToClip;

void main(void)
{
    gl_Position = cameraToClip * modelToCamera * position;
}
