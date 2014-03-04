#version 420

layout( location = 0 ) in vec3 inPosition;
//layout( location = 1 ) in vec3 inNormal; // currently not in use

uniform mat4 modelToCamera;
uniform mat4 cameraToClip;

//out vec3 vertexNormal;

void main(void)
{
    gl_Position = cameraToClip * modelToCamera * vec4( inPosition, 1.0 );
}
