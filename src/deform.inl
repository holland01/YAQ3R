
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

		float stripLen = glm::length( b2 - a2 );
		// Path trace the edges of our triangle defined by vertices a2 and b2
		const glm::vec3 e1( b0 - a0 );
		const glm::vec3 e2( c0 - b0 );

		float scalar = 0.0f;

		// if fmod( stripLen, sepLength ) != 0, then you need to come up with a resolution
		// which involves "fitting" as much of a triangle as possible into the last
		// very last iteration ( when walk == (stripLen - stepLength) )

		// Iterate along the edge and produce two tris per step
		for ( float walk = 0.0f; walk < stripLen; walk += stepLength )
		{
			glm::vec3 offset( a2 + e1 * scalar );
			
			glm::vec3 v1( offset );
			glm::vec3 v2( v1 + e1 );
			glm::vec3 v3( v2 + e2 );
			
			outVerts.push_back( vertexGenFn( v1, tessInfo ) );
			outVerts.push_back( vertexGenFn( v2, tessInfo ) );
			outVerts.push_back( vertexGenFn( v3, tessInfo ) );
			accumTris++;

			if ( walk < stripLen - stepLength )
			{
				outVerts.push_back( vertexGenFn( v3 + e1, tessInfo ) );				
				accumTris++;
			}
			else
			{ // Add a dummy vertex to produce a degenerate triangle so the strip doesn't get pwnt.
				outVerts.push_back( outVerts[ outVerts.size() - 1 ] );
			}

			scalar += 1.0f;
		}

		LTessellate_r( a2 + aToC, b2 + bToC );
	};

	LTessellate_r( a, b );
}