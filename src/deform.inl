
template < typename vertexType_t > static INLINE void TessellateTri( 
	std::vector< vertexType_t >& outVerts, 
	std::function< vertexType_t( const glm::vec3&, const tessellateInfo_t& ) > vertexGenFn, // distance to 
	const float amount, 
	const glm::vec3& a, 
	const glm::vec3& b, 
	const glm::vec3& c, 
	const glm::vec3& surfaceNormal )
{
	// TODO:
	// moving towards the centroid with each recursive call actually won't work: it causes major overlap in the triangles.
	// The solution to this would involve finding a direction which each vertex can gradually move which
	// doesn't cause any overlap. The step amount is definitely the scalar for each direction, however
	// the direction itself will probably be dependent on the edgeData_t which is constructed for each recurse

	using tessLambda_t = std::function< void( const glm::vec3& a2, const glm::vec3& b2, const glm::vec3& c2 ) >;
	const float oneThird = 1.0f / 3.0f;

	const glm::vec3 center( 
		( a.x + b.x + c.x ) * oneThird,
		( a.y + b.y + c.y ) * oneThird,
		( a.z + b.z + c.z ) * oneThird 
	);

	const float step = 1.0f / amount;
	const float distThreshold = step;

	glm::vec3 aToCenter( center - a );
	glm::vec3 bToCenter( center - b );
	glm::vec3 cToCenter( center - c );

	const glm::vec3 a0( a * step );
	const glm::vec3 b0( b * step );
	const glm::vec3 c0( c * step );

	tessellateInfo_t tessInfo;
	tessInfo.outerDistToCenter = ( glm::length( aToCenter ) + glm::length( bToCenter ) + glm::length( cToCenter ) ) * oneThird;

	// We use this as a means to move the inner triangle vertices gradually towards
	// the center of the outer triangle (defined by verts a, b, c)
	aToCenter *= step;
	bToCenter *= step;
	cToCenter *= step;

	tessLambda_t LTessellate_r = [ & ]( const glm::vec3& a2, const glm::vec3& b2, const glm::vec3& c2 )
	{
		float a2ToCenterDist = glm::length( center - a2 );
		float b2ToCenterDist = glm::length( center - b2 );
		float c2ToCenterDist = glm::length( center - c2 );

		tessInfo.innerDistToCenter = ( a2ToCenterDist + b2ToCenterDist + c2ToCenterDist ) * oneThird;

		if ( a2ToCenterDist < distThreshold && b2ToCenterDist < distThreshold && c2ToCenterDist < distThreshold )
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

		for ( int i = 0; i < 3; ++i )
		{
			// We subtract by this for each point to accomodate for the amount which the origin of
			// the edge offsets each vertex of the triangle; without this, the first
			// triangle for a given edge iteration will reside outside of the triangle. The subtraction
			// ensures the vertices move in the opposite direction with respect to the edge origin
			glm::vec3 diff( edges[ i ].start * step );

			for ( float walk = 0.0f; walk < edges[ i ].amount; walk += 1.0f )
			{
				glm::vec3 offset( edges[ i ].start + edges[ i ].subedge * walk );
			
				glm::vec3 v1( offset + a0 - diff );
				glm::vec3 v2( offset + b0 - diff );
				glm::vec3 v3( offset + c0 - diff );
			
				// Calculate indices before adding vertices, since the size of the buffer
				// is the index of v1 ( thirdBase follows a pattern based on the ordering of the edges of the main triangle,
				// which allows us to derive the last vertex properly )
				size_t secondBase = outVerts.size() + edges[ i ].offsetIndex;
				size_t thirdBase = outVerts.size() + edges[ ( i + 2 ) % 3 ].offsetIndex;

				outVerts.push_back( vertexGenFn( v1, tessInfo ) );
				outVerts.push_back( vertexGenFn( v2, tessInfo ) );
				outVerts.push_back( vertexGenFn( v3, tessInfo ) );
			
				// Don't append a second triangle if we're on the last iteration
				if ( walk < edges[ i ].amount - 1.0f )
				{
					outVerts.push_back( vertexGenFn( outVerts[ secondBase ].position, tessInfo ) ); 
					outVerts.push_back( vertexGenFn( outVerts[ secondBase ].position + edges[ i ].subedge, tessInfo ) );
					outVerts.push_back( vertexGenFn( outVerts[ thirdBase ].position, tessInfo ) );
				}
			}
		}

		LTessellate_r( a2 + aToCenter, b2 + bToCenter, c2 + cToCenter );
	};

	LTessellate_r( a, b, c );
}