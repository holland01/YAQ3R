
#include <cmath>
#include <glm/gtx/projection.hpp>

template < typename vertexType_t > static INLINE void TessellateTri( 
	std::vector< vertexType_t >& outVerts, 
	std::function< vertexType_t( const glm::vec3&, const tessellateInfo_t& ) > vertexGenFn,
	const float amount, 
	const glm::vec3& a, 
	const glm::vec3& b, 
	const glm::vec3& c, 
	const glm::vec3& surfaceNormal )
{
	auto LTriArea = []( const glm::vec3& v1, const glm::vec3& v2, const glm::vec3& v3 ) -> float
	{
		const glm::vec3 e1( v1 - v2 );
		const glm::vec3 e2( v3 - v2 );

		return 0.5f * glm::length( glm::cross( e1, e2 ) );
	};

	using tessLambda_t = std::function< void( const glm::vec3& a2, const glm::vec3& b2 ) >;

	const float aLength = glm::length( a );
	const float bLength = glm::length( b );

	float step = 1.0f / amount;

	glm::vec3 a0( a * step );
	glm::vec3 b0( b * step );
	glm::vec3 c0( c * step );

	glm::vec3 bToC( c0 - b0 );
	glm::vec3 aToC( c0 - a0 );
	
	float stepLength = glm::length( b0 - a0 );
	float subdivAmount = amount;
	float maxTris = LTriArea( a, b, c ) / LTriArea( a0, b0, c0 );

	tessellateInfo_t tessInfo;

	float accumTris = 0;

	auto LValidateClippedArea = []( const glm::vec3& top, const glm::vec3& bot, const glm::vec3& left ) -> bool
	{
		const glm::vec3& normal = left - bot;
		  
		return glm::length( normal ) >= glm::length( glm::proj( left - top, normal ) );
	};

	tessLambda_t LTessellate_r = [ & ]( const glm::vec3& a2, const glm::vec3& b2 )
	{
		if ( accumTris >= maxTris )
			return;

		const glm::vec3 aToB( b2 - a2 );

		float numStrips = glm::length( aToB ) / glm::length( b0 - a0 );
		float rem = numStrips - glm::floor( numStrips );
		numStrips -= rem;

		// Path trace the edges of our triangle defined by vertices a2 and b2
		const glm::vec3 e1( b0 - a0 );
		const glm::vec3 e2( c0 - a0 );
		const glm::vec3 bTop( a2 + aToB );
		const glm::vec3 bLeft( b2 + bToC );

		// if fmod( stripLen, sepLength ) != 0, then you need to come up with a resolution
		// which involves "fitting" as much of a triangle as possible into the last
		// very last iteration ( when walk == (stripLen - stepLength) )
		// Iterate along the edge and produce two tris per step

		bool remStripNeeded = false;
		float walk;
		for ( walk = 0.0f; walk < numStrips; walk++ )
		{
			glm::vec3 offset( a2 + e1 * walk );
			
			glm::vec3 v2( offset + e1 );
			glm::vec3 v3( offset + e2 );
			
			outVerts.push_back( vertexGenFn( offset, tessInfo ) );
			outVerts.push_back( vertexGenFn( v2, tessInfo ) );
			outVerts.push_back( vertexGenFn( v3, tessInfo ) );
			accumTris++;
			
			// If we're on the last iteration, verify we have enough 
			// room left in order to append another triangle;
			// a rem of 0 means that we indefinitely cannot fit another
			// triangle of normal size; otherwise, we perform one last check
			if ( walk >= numStrips - 1.0f )
			{
				if ( rem == 0.0f || !LValidateClippedArea( bTop, v2, bLeft ) )
				{
					// This obviously is irrelevant if rem is 0
					remStripNeeded = true;
					continue;
				}
			}
			else 
			{
				outVerts.push_back( vertexGenFn( v3, tessInfo ) );
				outVerts.push_back( vertexGenFn( v2, tessInfo ) );
				outVerts.push_back( vertexGenFn( v3 + e1, tessInfo ) );
			
				accumTris++;
			}
		}

		// There's a little bit left to work with,
		// so we add what we can
		if ( rem != 0.0f )
		{
			glm::vec3 v1( a2 + e1 * walk );

			glm::vec3 up = b2 - v1;
			glm::vec3 left = bLeft - v1;

			glm::vec3 v2( v1 + up );
			glm::vec3 v3( v1 + left );

			// Adjust to the other side if necessary, taking winding order into account 
			if ( remStripNeeded )
			{
				v1 = v2;
				v2 = v3 + up;
			}

			outVerts.push_back( vertexGenFn( v1, tessInfo ) );
			outVerts.push_back( vertexGenFn( v2, tessInfo ) );
			outVerts.push_back( vertexGenFn( v3, tessInfo ) );
			accumTris++;
		}

		LTessellate_r( a2 + aToC, bLeft );
	};

	LTessellate_r( a, b );
}