#pragma once

#include "common.h"


/*
===========================================

        Axially-Aligned Bounding Box

===========================================
*/

class AABB
{
public:

    AABB( void ); // Calls Empty() on default init

    AABB( const glm::vec3& max, const glm::vec3& min );

    AABB( const AABB& toCopy );

    ~AABB( void );

    AABB&       operator =( AABB toAssign );

    void        Add( const glm::vec3& p );

    void        Empty( void ); // Sets maxPoint to -pseudoInfinity, and minPoint to pseudoInfinity

    void        TransformTo( const AABB& box, const glm::mat4& transform ); // Finds the smallest AABB from a given transformation

    glm::vec3       GetMaxRelativeToNormal( const glm::vec3& normal ) const;

    glm::vec3       GetMinRelativeToNormal( const glm::vec3 &normal ) const;

    glm::vec3        Center( void ) const;

    glm::vec3        Size( void ) const;

    glm::vec3        Radius( void ) const;

    glm::vec3        Corner( int32_t index ) const;

    bool        IsEmpty( void ) const;

    bool        InPointRange( float k ) const;

    static void FromTransform( AABB& box, const glm::mat4& transform );

    static void FromPoints( AABB& box, const glm::vec3 p[], int32_t n );

    glm::vec3        maxPoint, minPoint;
};
