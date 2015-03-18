
#include <cmath>
#include <glm/gtx/projection.hpp>
#include <memory>

template < typename vertexType_t > static INLINE void TessellateTri( 
	std::vector< vertexType_t >& outVerts, 
	std::vector< triangle_t >& outIndices,
	std::function< vertexType_t( const glm::vec3& ) > vertexGenFn,
	const float amount, 
	const glm::vec3& a, 
	const glm::vec3& b, 
	const glm::vec3& c )
{
	auto LTriArea = []( const glm::vec3& v1, const glm::vec3& v2, const glm::vec3& v3 ) -> float
	{
		const glm::vec3 e1( v1 - v2 );
		const glm::vec3 e2( v3 - v2 );

		return 0.5f * glm::length( glm::cross( e1, e2 ) );
	};

	using tessLambda_t = std::function< void( const glm::vec3& a2, const glm::vec3& b2 ) >;

	float step = 1.0f / amount;

	// Use these as a basis for when either the a or b traversal vectors
	// have passed vertex c
	glm::vec3 aSig = glm::sign( c - a );
	glm::vec3 bSig = glm::sign( c - b ); 

	bool aPassed = false;
	bool bPassed = false;

	glm::vec3 bToC( ( c - b ) * step );
 	
	glm::vec3 aToC( ( c - a ) * step ); 
	aToC *= glm::length( bToC ) / glm::length( aToC );

	std::unique_ptr< baryCoordSystem_t > triCoordSys( new baryCoordSystem_t( a, b, c ) );

	auto LPassedC = [ &c ]( const glm::vec3& point, const glm::vec3& sig ) -> bool
	{
		return ( glm::sign( c - point ) != sig );
	};

	glm::vec3 a2( a );
	glm::vec3 b2( b );

	while ( a2 != c && b2 != c )
	{
		if ( glm::any( glm::isnan( a2 ) ) || glm::any( glm::isnan( b2 ) ) )
			break;

		// If either of these are set to true, then we'll have
		// triangles which exist outside of the parent tri.
		if ( LPassedC( a2, aSig ) || LPassedC( b2, bSig ) )
			break;

		glm::vec3 aToB( b2 - a2 );
		glm::vec3 aToBStep( aToB * step );
		
		// Path trace the edges of our triangle defined by vertices a2 and b2
		glm::vec3 end( a2 + aToB );

		float walk = 0.0f;
		float walkLength = 0.0f;
		float walkStep = glm::length( aToBStep ) / glm::length( aToB );
		float endLength = glm::length( aToB );
		
		while ( walkLength < endLength )
		{
			glm::vec3 v1( a2 + aToB * walk );
			glm::vec3 v2( v1 + aToBStep );
			glm::vec3 v3( v1 + aToC );
					
			// Clamp our second vertex at the top of the path
			// for this strip if it's out of bounds
			if ( !triCoordSys->IsInTri( v2 ) )
				v2 = a2 + aToB;

			// Clamp our third vertex to the next
			// starting point for b2 if it's out of bounds
			if ( !triCoordSys->IsInTri( v3 ) )
				v3 = b2 + bToC;

			size_t numVertices = outVerts.size();

			vertexType_t gv1 = vertexGenFn( v1 );
			vertexType_t gv2 = vertexGenFn( v2 );
			vertexType_t gv3 = vertexGenFn( v3 );

			triangle_t t1;

			auto v1Iter = std::find( outVerts.begin(), outVerts.end(), gv1 );
			if ( v1Iter == outVerts.end() )
			{
				outVerts.push_back( gv1 );
				t1.indices[ 0 ] = numVertices++;
			}
			else
			{
				t1.indices[ 0 ] = v1Iter - outVerts.begin(); 
			}

			auto v2Iter = std::find( outVerts.begin(), outVerts.end(), gv2 );
			if ( v2Iter == outVerts.end() )
			{
				outVerts.push_back( gv2 );
				t1.indices[ 1 ] = numVertices++;
			}
			else
			{
				t1.indices[ 1 ] = v2Iter - outVerts.begin(); 
			}

			auto v3Iter = std::find( outVerts.begin(), outVerts.end(), gv3 );
			if ( v3Iter == outVerts.end() )
			{
				outVerts.push_back( gv3 );
				t1.indices[ 2 ] = numVertices++;
			}
			else
			{
				t1.indices[ 2 ] = v3Iter - outVerts.begin(); 
			}

			outIndices.push_back( t1 );
			
			// Attempt a second triangle, providing v4
			// is within the bounds
			glm::vec3 v4( v3 + aToBStep );
			if ( !triCoordSys->IsInTri( v4 ) )
				goto end_iteration;

			{
				vertexType_t gv4 = vertexGenFn( v4 );
				auto v4Iter = std::find( outVerts.begin(), outVerts.end(), gv4 );

				triangle_t t2 = 
				{
					{
						t1.indices[ 2 ],
						t1.indices[ 1 ],
						0
					}
				};
			
				if ( v4Iter == outVerts.end() )
				{
					outVerts.push_back( gv4 );
					t2.indices[ 2 ] = numVertices;
				}
				else
				{
					t2.indices[ 2 ] = v4Iter - outVerts.begin();
				}

				outIndices.push_back( t2 );
			}

end_iteration:
			walk += walkStep;
			walkLength = glm::length( aToB * walk );
		}
		
		a2 += aToC;
		b2 += bToC;
	}
}