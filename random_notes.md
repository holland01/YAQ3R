**3/6/16**

For the glTextures array that's owned by the BSPRenderer class, I'm not sure if that actually needs to be a field.
Either way, one possible method for maintaining consistency with the texture indices (that aren't already used by effect shaders)
while still eliminating redundancy and lowering memory usage for both GPU and CPU, might be using a byte buffer which is "blanked"
out with individual bytes in areas where texture indices aren't actually relevant, and in areas where they *are* relevant, storing
a serialized struct at the appropriate offset. Given the nature of the renderer, any indexing into this data structure would implicitly
guarantee a proper lookup such that no "undefined" or "blanked" out memory was accessed. If the std::unordered_map implementation is at risk
of allocating more memory than desired, even if only a few items are stored, this really could be desirable.

The end result would be storing the texture offsets in the atlas as serializable memory, which pretty much implies ditching
the glm vector usage in this particular case.

Dummy textures which are made shouldn't even be allocated on the GPU; there should be a means of finding them when the atlas
is being constructed, ignoring them, and therefore not even adding their data to the atlas itself. This obviously prevents
unnecessary glTexImage2D calls. If anything, only one dummy texture should be allocated, and any indices which require a dummy should be
marked in the atlas data so that a bind to a dummy can simply be mapped when an arbitrary index is passed.

**3/7/16**

#### Textures

Apparently using GL_BGRA as an input format for textures can prevent having to do intermediate software memory transfers on the GPU...

#### Effect Shaders

Definitely add the following function to `shaderStage_t`:

`const Program& GetProgram( void ) const { return *GQueryProgram( program ); }`

This will help in replacing all of the calls in the renderer.

#### Performance

Running the renderer using Intel drivers gives me 30 FPS at best and over 160 MB of RAM per frame.

**Update** with O2, Intel drivers on Windows give about 120 FPS, which is pretty solid. The high RAM usage is still prevelant,
but apparently the HD 4600 only has 128 MB of dedicated VRAM. This may explain the dfferences in system RAM usage being reported between Linux and Windows.

On Linux I get 60 MB of ram and 60 FPS with a) intel drivers and b) less convenient features.

I'm thinking the issue might be due to dependencies which are linked in via MSVC, and possibly other things like meta data
for debugging which is applied in VS at runtime.


**On programs ( this should be done AFTER removal of duplicate programs )**

One thing which really needs to happen is that there should be strictly one single buffer for the lazy uniform upload
that happens on program binding for the `Program` class. Each element in that buffer needs to be a union, which maps
to the different primitive types (int, mat4, vec2, etc etc) using strictly primitive data and nothing else (i.e., no GLM).

It *should* be possible to just use the uniform locations as indices into the buffer for accessing the respective elements,
that is assuming that a group of uniforms will have contiguous indices (assuming no -1's exist).

Either way, the buffer should just be allocated once - this will prevent the need for it to be "cleared" each time
the program is released. To avoid always writing every frame, you can design the struct like so:

```
enum uniformType_t
{
	UNIFORM_INT = 0,
	...
	more stuff
	...
	UNIFORM_VEC2,
	UNIFORM_VEC3,
	UNIFORM_VEC4,
	...
	UNIFORM_MAT4
};

struct uniform_t
{
	union data {
		int i;

		...
		more stuff
		...

		float vec2[ 2 ];
		float vec3[ 3 ];
		float vec4[ 4 ];
		...
		float mat4[ 16 ];
	};
	uniformType_t type;
	bool needs_write;
};
```

(**NOTE** Using a buffer of bytes which are of variable length (depending on the
byte width for the type needed may be more advantageous than
using a full blown union, given that it would eliminate unnecessary memory usage
for values like integers, which in the union instance would still require 64 bytes of memory
to be taken up, as a means to support 4x4 matrices).

simply send the upload on program bind, and then make sure you set needs_write to false
for every newly uploaded element.

This is totally a C approach. It's also a much simpler approach than what would be written in idiomatic C++,
and also far more reliable (in this case).

**3/8/16**

#### Profiling

Definitely need to find a means of profiling on Linux. intel-gpu-tools seems like a good
bet, however it requires packages which are too ahead of what's available for my
current Linux distro. It may be time to upgrade, if nothing else is available.

#### Splitting (Textures)

Use the grid structure as a means to bind staged slots approapriately. Textures should
generally be split when hardware limitations don't allow them to used as a whole. Some
images will be part of one grid while others will be apart of another.

**3/9/16**

#### Splitting (Textures)

* Indexing/Lookup scheme for grids and the stOffset{Start|End} parameters, using the grid's {x, y}{Start|End} parameters
is somewhat inefficient; there's likely a better way to find the proper coordinate, using the semantics of the grid's
scheme itself, in additiont to some implicit logarithmic/quadtree-esque properties.

* The GMakeTexture function has a section of code which determines the atlas size. Make the following modifications:

	- Abstract out the size determining function so that the dimensions can be recalculated if the atlas needs to be subdivided;
		each subdivision/grid within the atlas will have separate canvas/slot parameters.

	- Sort the images by their size (consider the magnitude of their width and height, or sort by width first then height as a secondary for widths which
	are equivalent. [1]). You can swap out the std::for_each with an std::sort, and still find the max dimensions that way.

	- Move the abstracted size compute function so that it's called for every sub-atlas created.

* You'll probably have to grow and shrink different subdivisions to ensure that there's enough room for each atlas. The sort by dimension should help with this.

[1] e.g., for the following images:

( 64, 128 ); ( 64, 256 ) | ( 128, 256 ); ( 128, 512 ) | ... ( w, h0 ); ( w, h1 ); .... ; ( w, hn )

**3/10/16**

#### Splitting (Textures) -> Placement

Before splitting is to take place, it's clear that a mechanism needs to be devised which will encourage textures of different sizes to be placed
more adjacent to each other. For a given slot, which represents the maximum dimensions of an image in a corresponding group, you want to fit as many smaller
images as possible into that slot. In other words, slots which only contain a single image should be avoided as much as possible.

The best way to achieve this involves, after sorting the images by the vector length of their dimensions:
	* Iterating over each image
	* Checking every other image and assessing whether or not it can fit inside the current slot our iterator represents
	* Keeping track of slots which are emptied and images which have been moved (so they aren't affected in a future in iteration, after they've been appointed a different slot)
	* Using a bounds mechanism for each image to aid in determining whther or not the candidate image can "fit" inside the slot, against other images
	of varied dimensions, location, and orientation.
		- For this, it'd be good to use a perimeter generation mechanism as a means to simply find a good fit for the incoming candidate:
		- Be sure a given group of images for a single slot is sorted by increasing x-values - this will guarantee that, when perimeter is constructed,
		we'll always be moving towards the next image in an increasing manner.

			The perimeter construction mechanism should be something along the lines of:
				start0 = slotOrigin
				end0 = (start0_x, max0_y)
				start1 = end0
				end1 = (min1_x, end1_y)
				start2 = end1
				end2 = (start2_x, max1_y)

				Some important things to think about are:
					how do we know which y coordinate to choose? X is obvious, because the list is sorted by x values.
					Using the max-y-value for each bounding box appears to be reasonable method.

				Anyway, the perimeter generation method is useful because the box of the image which we want to insert
				cannot penetrate any area of that line.

**3/13/16**

So far, there has yet to be a minimum distance which doesn't meet the requirements of being greater than or equal
to the dimensions of the bounds being moved. However, there's still problems.

A couple issues are:

a) some shapes will, for some reason, just stay inside the origin of the owning region bounds.
Something is probably wrong with the collision detection, considering that no invalid dimension/distance
issues have been found yet.

b) Some resolved primitives will appear to penetrate the boudns of the atlas itself; this may
be due to too high of an offset being applied in the collision resolution loop.

**3/15/16**

So far, so good. Everything is much, much simpler now...as well as more efficient.

However, the Railgun_Arena test has an x-offset for glTexSubImage being mapped outside of the
dimensions of the texture memory; the x-offset's value is 512 (After the subtraction by half-width is made)
and the width of the texture itself (the canvasParams) is 512...

This is pretty bad. However, doubling the initial width generated has appeared
to at least bypass the GL error that's thrown.

Another problem emerges, though: the implementation will not work for sets of images which all have the same dimensions.

For Quake 3 maps, this definitely means that lightmaps cannot thrive on this setting. It may
therefore be advantagous to produce a an atlas via a much simpler, linear means. In other words,
forgoing the tree search/setup and finding a simple square dimensions among the amount of images,
each of which is 256 * 256 texels in size (or whatever the standard size is for lightmaps).

You can determine which approach to take simply by assessing whether or not there is any
significant variation between the dimensions for all of the images. If all of the dimensions
are the exact same, just find a square from a 1D size for a texture buffer and then
fill it linearly.

Also, the final addition to the origin at the end of the TreePoint function, after the height
has been applied, looks as follows:

```
origin.x += width * 0.5f
origin.y += height * 0.5f
```

Despite being useful in the original unit test, this now is actually completely useless:
in the GenSubdivision call, these half width/height offsets are removed, placing the origin
back where it originally was - these may as well be ditched altogether.

