#include "math_util.h"

glm::vec3 GetBoundsCenter( glm::ivec3& max, glm::ivec3& min )
{
    glm::vec3 vmax( ( float ) max.x, ( float ) max.y, ( float ) max.z );
    glm::vec3 vmin( ( float ) min.x, ( float ) min.y, ( float ) min.z );

    return vmin + ( vmax - vmin ) / 2.0f;
}

glm::fquat MakeQuat( float angRad, const glm::vec3& axis )
{
    float halfTheta = angRad * 0.5f;
    float sinTheta = glm::sin( halfTheta );

    float w = glm::cos( halfTheta );

    float x = sinTheta * axis.x;
    float y = sinTheta * axis.y;
    float z = sinTheta * axis.z;

    return glm::fquat( w, x, y, z );
}

void OrthoNormalBasisFromForward( const glm::vec3& forwardDir, glm::mat3& basis )
{
	const glm::vec3 up( 0.0f, 1.0f, 0.0f );
	const glm::vec3 right( 1.0f, 0.0f, 0.0f );

	float dotUp = glm::abs( glm::dot( forwardDir, up ) );
	float dotRight = glm::abs( glm::dot( forwardDir, right ) );

	if ( glm::min( dotUp, dotRight ) == dotUp )
	{
		basis[ 0 ] = glm::cross( forwardDir, up );
		basis[ 1 ] = glm::cross( forwardDir, -basis[ 0 ] ); // right hand rule implies that forward X left will produce proper up
	}
	else
	{
		basis[ 1 ] = glm::cross( forwardDir, -right );
		basis[ 0 ] = glm::cross( forwardDir, basis[ 1 ] );
	}

	basis[ 2 ] = forwardDir;
}
