
#include <cmath>

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

	tessLambda_t LTessellate_r = [ & ]( const glm::vec3& a2, const glm::vec3& b2 )
	{
		if ( accumTris >= maxTris )
			return;

		float numStrips = glm::length( b2 - a2 ) / glm::length( b0 - a0 );
		float rem = numStrips - glm::floor( numStrips );
		numStrips -= rem;

		// Path trace the edges of our triangle defined by vertices a2 and b2
		const glm::vec3 e1( b0 - a0 );
		const glm::vec3 e2( c0 - b0 );

		float walk;

		// if fmod( stripLen, sepLength ) != 0, then you need to come up with a resolution
		// which involves "fitting" as much of a triangle as possible into the last
		// very last iteration ( when walk == (stripLen - stepLength) )

		// Iterate along the edge and produce two tris per step
		for ( walk = 0.0f; walk < numStrips; walk++ )
		{
			glm::vec3 offset( a2 + e1 * walk );
			
			glm::vec3 v2( offset + e1 );
			glm::vec3 v3( offset + e2 );
			
			outVerts.push_back( vertexGenFn( offset, tessInfo ) );
			outVerts.push_back( vertexGenFn( v2, tessInfo ) );
			outVerts.push_back( vertexGenFn( v3, tessInfo ) );
			accumTris++;

			// Ensure we're not on the last iteration, otherwise we have
			// a quad sticking out from the triangle edge
			if ( walk < numStrips - 1.0f )
			{
				outVerts.push_back( vertexGenFn( v3, tessInfo ) );
				outVerts.push_back( vertexGenFn( v3 + e1, tessInfo ) );
				outVerts.push_back( vertexGenFn( v2, tessInfo ) );
				accumTris++;
			}
		}

		// There's a little bit left to work with,
		// so we add what we can
		if ( rem != 0.0f )
		{
			glm::vec3 offset( a2 + e1 * walk );

			glm::vec3 v2( offset + e1 * rem );
			glm::vec3 v3( offset + e2 );

			outVerts.push_back( vertexGenFn( offset, tessInfo ) );
			outVerts.push_back( vertexGenFn( v2, tessInfo ) );
			outVerts.push_back( vertexGenFn( v3, tessInfo ) );
		}

		LTessellate_r( a2 + aToC, b2 + bToC );
	};

	LTessellate_r( a, b );
}