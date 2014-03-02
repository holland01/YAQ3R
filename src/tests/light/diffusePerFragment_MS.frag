#version 420

smooth in vec4 diffuseColor;
smooth in vec3 vertexNormal;
smooth in vec3 modelPosition;

uniform vec3 modelLightPos;
uniform vec4 lightIntensity;
uniform vec4 ambientIntensity;

out vec4 fragment;

void main(void)
{
    vec3 dirToLight = normalize( modelLightPos - modelPosition );

    float cosAngIncidence = dot( normalize( vertexNormal ), dirToLight );
    cosAngIncidence = clamp( cosAngIncidence, 0, 1 );

    fragment = ( diffuseColor * lightIntensity * cosAngIncidence ) + ( diffuseColor * ambientIntensity );
    fragment = diffuseColor;
}
