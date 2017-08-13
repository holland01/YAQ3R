## Perf

### 5/4/2017

#### Frustum

define two axes:
	* Xv: viewport X axis
	* Yv: viewport Y axis

The frustum is in world space, so what you want to do is map these axes (originally in screen space)
so that their respective normals (applied to both ends of each axis) extend towards their
frustum plane and thus intersect the plane itself. 

From there, you can use this intersection point to define the beginning of a subplane which extends within the direction of the normal
outward to infinity and is parallel with the world-relative XY plane normal of the frustum.

This should help with clipping areas that are wide open.


#### State management

Should have a client-side means of keeping track of all used parameters. Basically any opengl function call
which changes a particular state parameter found in the implementation should be kept track of and stored.

Maybe use a stack based mechanism for each param. You want this primarily because it reduces the need for
meta function calls like glIsEnabled or glGetIntegerv, which may or may not have an impact on performance.


