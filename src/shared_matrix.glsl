#version 420

layout(std140) uniform GlobalMat4
{
    mat4 modelView;
    mat4 projection;
};

vec4 vertexToClip( in vec3 position )
{
    return projection * modelView * vec4( position, 1.0 );
}

layout(std140) uniform GlobalMat3
{
    mat3 modelViewNormal;
};

vec3 normalToEye( in vec3 normal )
{
    return modelViewNormal * normal;
}
