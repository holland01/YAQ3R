

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

	using tessLambda_t = std::function< void( const glm::vec3& a2, const glm::vec3& b2, const glm::vec3& c2 ) >;

	const float step = 1.0f / amount;

	const glm::vec3 a0( a * step );
	const glm::vec3 b0( b * step );
	const glm::vec3 c0( c * step );

	const int maxTris = ( int )glm::ceil( LTriArea( a, b, c ) / LTriArea( a0, b0, c0 ) );

	tessellateInfo_t tessInfo;

	int accumTris = 0;

	tessLambda_t LTessellate_r = [ & ]( const glm::vec3& a2, const glm::vec3& b2, const glm::vec3& c2 )
	{
		if ( accumTris >= maxTris )
			return;

		const float numAB( glm::floor( glm::length( b2 - a2 ) / glm::length( b0 - a0 ) ) );
		const float numBC( glm::floor( glm::length( c2 - b2 ) / glm::length( c0 - b0 ) ) );
		const float numCA( glm::floor( glm::length( a2 - c2 ) / glm::length( a0 - c0 ) ) );

		// Path trace the edges of our triangle defined by vertices a2, b2, and c2
		struct edgeData_t
		{
			float amount;
			glm::vec3 subedge;
			glm::vec3 start;
		} 
		edges[ 3 ] = 
		{
			{ numAB, b0 - a0, a2 },
			{ numBC, c0 - b0, b2 },
			{ numCA, a0 - c0, c2 }
		};

		for ( int i = 0; i < 3; ++i )
		{
			tessInfo.edgeIndex = i;

			// Iterate along the edge and produce two tris per step
			for ( float walk = 0.0f; walk < edges[ i ].amount; walk += 1.0f )
			{
				glm::vec3 offset( edges[ i ].start + edges[ i ].subedge * walk );
			
				glm::vec3 v1( offset );
				glm::vec3 v2( v1 + edges[ i ].subedge );
				glm::vec3 v3( v2 + edges[ ( i + 1 ) % 3 ].subedge );
			
				outVerts.push_back( vertexGenFn( v1, tessInfo ) );
				outVerts.push_back( vertexGenFn( v2, tessInfo ) );
				outVerts.push_back( vertexGenFn( v3, tessInfo ) );
				outVerts.push_back( vertexGenFn( v3 + edges[ i ].subedge, tessInfo ) );				
				
				accumTris += 2;
			}
		}

		LTessellate_r( a2 + ( c0 - a0 ) + edges[ 0 ].subedge, b2 + ( a0 - b0 ) + edges[ 1 ].subedge, c2 + ( b0 - c0 ) + edges[ 2 ].subedge );
	};

	LTessellate_r( a, b, c );
}