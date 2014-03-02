#version 420

in vec3 inPosition;
in vec4 inColor;
in vec3 inNormal;

uniform mat4 modelToCamera;
uniform mat4 cameraToClip;

smooth out vec4 diffuseColor;
smooth out vec3 modelPosition;
smooth out vec3 vertexNormal;

void main(void)
{
    gl_Position = cameraToClip * modelToCamera * vec4( inPosition, 1.0 );

    modelPosition = inPosition;
    diffuseColor = inColor;
    vertexNormal = inNormal;
}
