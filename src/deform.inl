

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

	const float step = 1.0f / amount;

	const glm::vec3 a0( a * step );
	const glm::vec3 b0( b * step );
	const glm::vec3 c0( c * step );

	const float maxTris = glm::ceil( LTriArea( a, b, c ) / LTriArea( a0, b0, c0 ) );

	tessellateInfo_t tessInfo;

	float accumTris = 0;

	tessLambda_t LTessellate_r = [ & ]( const glm::vec3& a2, const glm::vec3& b2 )
	{
		if ( accumTris >= maxTris )
			return;

		const float numAB( glm::ceil( glm::length( b2 - a2 ) / glm::length( b0 - a0 ) ) );

		// Path trace the edges of our triangle defined by vertices a2 and b2
		const glm::vec3 e1( b0 - a0 );
		const glm::vec3 e2( c0 - b0 );

		for ( int i = 0; i < 1; ++i )
		{
			tessInfo.edgeIndex = i;

			// Iterate along the edge and produce two tris per step
			for ( float walk = 0.0f; walk < numAB; walk += 1.0f )
			{
				glm::vec3 offset( a2 + e1 * walk );
			
				glm::vec3 v1( offset );
				glm::vec3 v2( v1 + e1 );
				glm::vec3 v3( v2 + e2 );
			
				outVerts.push_back( vertexGenFn( v1, tessInfo ) );
				outVerts.push_back( vertexGenFn( v2, tessInfo ) );
				outVerts.push_back( vertexGenFn( v3, tessInfo ) );
				accumTris++;

				if ( walk < numAB - 1.0f )
					outVerts.push_back( vertexGenFn( v3 + e1, tessInfo ) );				
				else // Add a dummy vertex to produce a degenerate triangle so the strip doesn't get pwnt.
					outVerts.push_back( outVerts[ outVerts.size() - 1 ] );

				accumTris++;
			}
		}

		LTessellate_r( a2 + ( c0 - a0 ), b2 + ( c0 - b0 ) );
	};

	LTessellate_r( a, b );
}