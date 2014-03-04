#version 420

smooth in vec4 diffuseColor;

smooth in vec3 vertexNormal;
smooth in vec3 modelPosition;

uniform mat3 normalTransform;

uniform vec3 halfVector;
uniform vec3 modelLightPos;

uniform vec4 lightIntensity;
uniform vec4 ambientIntensity;

uniform float shininess;
uniform float strength;

out vec4 fragment;

float ComputeAngleOfIncidence( in vec3 normal, in vec3 direction )
{
    float angleOfIncidence = dot( normal, direction );
    return clamp( angleOfIncidence, 0.0, 1.0 );
}

void main(void)
{
    vec3 normalizedVertexNormal = normalize( normalTransform * vertexNormal );

    vec3 dirToLight = normalize( modelLightPos - modelPosition );

    float specular = ComputeAngleOfIncidence( normalizedVertexNormal, normalize( halfVector ) );
    float diffuse = ComputeAngleOfIncidence( normalizedVertexNormal, dirToLight );

   if ( diffuse == 0.0 )
        specular = 0.0;
    else
        specular = pow(specular, shininess);

    vec3 lightScattered = lightIntensity.rgb * diffuse + ambientIntensity.rgb;
    vec3 lightReflected = lightIntensity.rgb * specular * strength;

    vec3 rgbColor = min( diffuseColor.rgb * ( lightScattered + lightReflected ), vec3( 1.0 ) );

    fragment = vec4( rgbColor, 1.0 );
}
