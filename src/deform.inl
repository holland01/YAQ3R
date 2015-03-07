

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

	// TODO:
	// moving towards the centroid with each recursive call actually won't work: it causes major overlap in the triangles.
	// The solution to this would involve finding a direction which each vertex can gradually move which
	// doesn't cause any overlap. The step amount is definitely the scalar for each direction, however
	// the directions themselves will probably be dependent on the edgeData_t which is constructed for each recurse

	using tessLambda_t = std::function< void( const glm::vec3& a2, const glm::vec3& b2, const glm::vec3& c2 ) >;
	const float oneThird = 1.0f / 3.0f;

	const float step = 1.0f / amount;
	const float distThreshold = step;

	const glm::vec3 a0( a * step );
	const glm::vec3 b0( b * step );
	const glm::vec3 c0( c * step );

	const float maxTris = 256;

	tessellateInfo_t tessInfo;

	int triCount = 0;

	tessLambda_t LTessellate_r = [ & ]( const glm::vec3& a2, const glm::vec3& b2, const glm::vec3& c2 )
	{
		if ( triCount >= maxTris )
			return;

		const float numAB( glm::floor( glm::length( b2 - a2 ) / glm::length( b0 - a0 ) ) );
		const float numBC( glm::floor( glm::length( c2 - b2 ) / glm::length( c0 - b0 ) ) );
		const float numCA( glm::floor( glm::length( a2 - c2 ) / glm::length( a0 - c0 ) ) );

		// Path trace the edges of our triangle defined by vertices a2, b2, and c2
		struct edgeData_t
		{
			float amount;
			int offsetIndex; // an index to the vertex which we use as a basis for the second triangle
			glm::vec3 subedge;
			glm::vec3 start;
		} 
		edges[ 3 ] = 
		{
			{ numAB, 2, b0 - a0, a2 },
			{ numBC, 0, c0 - b0, b2, },
			{ numCA, 1, a0 - c0, c2 }
		};

		const bool isFirst = a2 == a && b2 == b && c2 == c;

		for ( int i = 0; i < 3; ++i )
		{
			// NOTE: This is for the set of triangles on the outermost edges only.
			// we subtract by this for each point to accomodate for the amount which the origin of
			// the edge offsets each vertex of the triangle; without this, the first
			// triangle for a given edge iteration will reside outside of the triangle. The subtraction
			// ensures the vertices move in the opposite direction with respect to the edge origin
			glm::vec3 diff( isFirst ? edges[ i ].start * step : glm::vec3( 0.0f ) );

			tessInfo.edgeIndex = i;

			for ( float walk = 0.0f; walk < edges[ i ].amount; walk += 1.0f )
			{
				glm::vec3 offset( edges[ i ].start + edges[ i ].subedge * walk );
			
				glm::vec3 v1( offset );
				glm::vec3 v2( v1 + edges[ i ].subedge );
				glm::vec3 v3( v2 + edges[ ( i + 1 ) % 3 ].subedge );
			
				// Calculate indices before adding vertices, since the size of the buffer
				// is the index of v1 ( thirdBase follows a pattern based on the ordering of the edges of the main triangle,
				// which allows us to derive the last vertex properly )
				size_t secondBase = outVerts.size() + edges[ i ].offsetIndex;
				size_t thirdBase = outVerts.size() + edges[ ( i + 2 ) % 3 ].offsetIndex;

				outVerts.push_back( vertexGenFn( v1, tessInfo ) );
				outVerts.push_back( vertexGenFn( v2, tessInfo ) );
				outVerts.push_back( vertexGenFn( v3, tessInfo ) );

				triCount++;
			
				// Don't append a second triangle if we're on the last iteration
				if ( walk < edges[ i ].amount - 1.0f )
				{
					outVerts.push_back( vertexGenFn( outVerts[ secondBase ].position, tessInfo ) ); 
					outVerts.push_back( vertexGenFn( outVerts[ secondBase ].position + edges[ i ].subedge, tessInfo ) );
					outVerts.push_back( vertexGenFn( outVerts[ thirdBase ].position, tessInfo ) );
				
					triCount++;
				}
			}
		}

		LTessellate_r( a2 + ( c0 - a0 ) + edges[ 0 ].subedge, b2 + ( a0 - b0 ) + edges[ 1 ].subedge, c2 + ( b0 - c0 ) + edges[ 2 ].subedge );
	};

	LTessellate_r( a, b, c );
}