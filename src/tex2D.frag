#version 130

in vec2 frag_UV;
in vec3 frag_Normal;

uniform sampler2D sampler;

uniform vec3 lightPosition;
uniform vec4 lightColor;

uniform float lightIntensity;

out vec4 out_Fragment;

vec3 normalToEye( in vec3 normal );

vec3 project( in vec3 src, in vec3 dest )
{
    float cosTheta = dot( src, dest ) / ( length( src ) * length( dest ) );

    return dest * ( ( length( src ) * cosTheta ) / length( dest ) );
}

void main(void)
{
    vec3 lightPosPrime = normalToEye( lightPosition );

    float diffuse = acos( dot( lightPosPrime, frag_Normal ) / length( lightPosPrime ) * length( frag_Normal ) );

    vec4 texSamp = texture( sampler, frag_UV );
    vec4 light = lightColor * diffuse * lightIntensity;
    out_Fragment = max( texSamp, texSamp * light );
}
