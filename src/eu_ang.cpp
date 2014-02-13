#include "eu_ang.h"


EuAng::EuAng( void )
    : pitch( 0 ), yaw( 0 ), roll( 0 )
{
}

EuAng::EuAng( float p, float y, float r )
    : pitch( p ),
      yaw( y ),
      roll( r )
{
}

glm::vec3 EuAng::ToVector( bool inRadians ) const
{
    float y, p;

    if ( inRadians )
    {
        y = glm::radians( yaw );
        p = glm::radians( pitch );
    }
    else
    {
        y = yaw;
        p = pitch;
    }

    return glm::vec3(
                glm::cos( y ) * glm::cos( p ),
                glm::sin( p ),
                glm::sin( y ) * glm::sin( p )
            );
}

void EuAng::Normalize( void )
{
    if ( pitch > 89.9f )
    {
        pitch -= 180.0f;
    }
    else if ( pitch < -89.9f )
    {
        pitch += 180.0f;
    }

    if ( yaw > 180.0f )
    {
        yaw -= 360.0f;
    }
    else if ( yaw < -180.0f )
    {
        yaw += 360.0f;
    }
}

