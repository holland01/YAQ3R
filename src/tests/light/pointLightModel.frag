#version 420

in vec3 vertexNormal;
in vec3 modelPosition;

uniform vec4 diffuseColor;
uniform vec4 lightIntensity;
uniform vec4 ambientIntensity;

uniform vec3 modelLightPos;

out vec4 fragment;

void main(void)
{
    vec3 dirToLight = modelPosition - modelLightPos;

    float cosAngIncidence = dot( vertexNormal, dirToLight );
    //cosAngIncidence = clamp( cosAngIncidence, 0.0, 1.0 );

    fragment =  ( diffuseColor * lightIntensity * cosAngIncidence ) + ( diffuseColor * ambientIntensity );
}
