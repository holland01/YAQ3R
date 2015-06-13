#version 420

smooth in vec3 frag_Position;
smooth in vec3 frag_Normal;

uniform sampler2D fragRadianceSampler;
uniform vec4 fragTargetPlane;
uniform vec3 fragSurfaceNormal;
uniform vec2 fragMin; // xz
uniform vec2 fragMax; // xz

layout( location = 0 ) out vec4 fragment;

const float gamma = 1.0 / 2.2;

struct Ray
{
	vec3 dir;
	vec3 origin;
};

void main( void )
{
	Ray r;
	r.origin = frag_Position;
	vec3 planeNormal = fragTargetPlane.xyz;

	float distanceNumerator = fragTargetPlane.w - dot( r.origin, planeNormal );

	vec4 color = vec4( 0.0 );
	for ( float phi = 0.0; phi <= 1.5707; phi += 0.1 )
	{
		for ( float theta = 0.0; theta <= 3.14159; theta += 0.1 )
		{
			ray.dir = vec3( cos( theta ) * cos( phi ), sin( phi ), cos( phi ) * sin( theta ) );

			float cosAngRay = dot( ray.dir, planeNormal );

			if ( cosAngRay == 0.0 )
			{
				continue;
			}
			
			// Compute distance from origin to sample point
			float t = distanceNumerator / cosAngRay; 
			
			// Measure sample's x and z coordinates against fragMin and fragMax's 
			// values. If the x and z coordinates fall outside the range of fragMin's or fragMax's,
			// where fragMin and fragMax's y value represents z in the xz plane,
			// then this is not a valid sample.

			// Otherwise, continue the use of the integration function: http://www.cs.virginia.edu/~jdl/bib/appearance/analytic%20models/schlick94b.pdf
			vec3 sample = ray.origin + ray.dir * t;
		}
	}
}