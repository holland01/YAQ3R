# Journal

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

On Linux I get 60 MB of ram and 60 FPS with a) intel drivers and b) less convenient features.

I'm thinking the issue might be due to dependencies which are linked in via MSVC, and possibly other things like meta data
for debugging which is applied in VS at runtime.

**Update** with O2, Intel drivers on Windows give about 120 FPS, which is pretty solid. The high RAM usage is still prevelant,
but apparently the HD 4600 only has 128 MB of dedicated VRAM. This may explain the dfferences in system RAM usage being reported between Linux and Windows.

#### Shader Programs ( this still needs to be done )

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


**3/17/15**

#### 12 AM
_______________________

Fixed issues with texture dimensions and the atlas layout for lightmaps.
Also refactored out BSPRenderer::LoadMainImages() into its own GU_LoadMainTextures function, which resides alongside GU_LoadShaderTextures.
The problem is that, even though the renderer runs without any errors, the sampling itself (which is being performed in the fragment shader) appears
to be relying on improperly computed texture coordinates. The end result, regardless of the cause, is incorrect.

Some theories as to why this is happening:

	- something is being missed in the sampling calculations (unlikely, but it can't be ruled out).
	- BSP texture indices aren't properly mapped to the texture module's internal structure (definitely possible).
	- Floating point issues with computing the proper offsets (unlikely, but it can't be ruled out).
	- the Main texture images aren't being properly bound; as a result, only lightmaps are being rendered (???).

TODO ( in order, starting from the first ):

	- In the test_textures module, you want to write out the necessary subroutines so that you can switch between rendering the entire atlas
	or an individual image from that atlas, using the up key. The left and right keys can scroll through individual images; we simply use an index
	which me modify per left/right keypress. Examine the results of these individual renders, as they may provide some significant insight.

	- In the actual renderer, double check the BSP texture indices and how they're mapped to the internal texture/atlas structure.

	- In the actual renderer, double check the render path (from the very top, in BSPRenderer::RenderPass()) and verify that the correct textures are being bound,
	along with things like expected blend modes, etc. being properly set.

#### 11 PM
_______________________

The problem was due to the imageScaleRatio not being set according to the image's dimensions. Given the fact
that the texture coordinates are in [0, 1), and that there was no actual indication of the image's width
and height in the computations, a very small portion of the image would be mapped.

So, in short, the first theory was correct (it's good it wasn't ruled out...).

Ultimately, though, this method is much more memory efficient in comparison to the older method.

This map would cause the renderer to crash when generating the texture data:

![atlas2](https://cloud.githubusercontent.com/assets/911971/13870664/f380d77c-ec9a-11e5-9745-207ee8741b2a.png)

However, there's still at least one issue: the support for NPOT images is non-existent, and this seems to

cause problems in some maps which heavily rely on NPOT images. The issue can be seen through

two separate entries in the texture atlas for q3tourney2.bsp: one image has dimensions 256 x 1553.

This image in particular overlaps other images, which has POT dimensions:

![atlas](https://cloud.githubusercontent.com/assets/911971/13870666/f3821a92-ec9a-11e5-8fee-081cabe2e503.png)

And here are the results in action:

![atlas_wtf](https://cloud.githubusercontent.com/assets/911971/13870665/f38171be-ec9a-11e5-9fad-6a2460082d34.png)

So, if the issue is the result of NPOT images being used, it may be a good idea to store two

separate widths in `gImageParams_t`: one for OpenGL, and the other for simple metadata.

That said, it's absolutely valid to store the width and the height in a single 32-bit integer,
rather than two separate 32-bit integers: 65,535 is obviously a ludicrous image dimension
for this scenario.

So, pack both dimensions in 4 bytes; and make two separate integers: one for GL and one for
whatever else. The GL one will just be the next rounded up Power of Two.

In the atlas generation tree (and possibly the uniform generation method as well), it will
be important to sort the images by their GL dimensions, and not their original sizes.

However, in the glTexSubImage function, and in any other instance where the actual image data

is being read, it should be obvious that the rounded GL dimensions can't be used...

Anyway, here's some quick benchmarks which compare the two atlas gen methods:

#### Branch: emscripten (old texture atlas generation)

**Map: Railgun_Arena.bsp**
_____________________________________

* Initial FPS: ~ [52]

* Shader Texture Texels: 4,194,304

* Main Texture Texels: 16,777,216

* Lightmap Texels: 1,048,576

* Total Texels: 22,020,096

* Total Bytes: 88,080,384

**Map: q3tourney2.bsp**
_____________________________________

* Notes:

	- crashes: Main Texture Texels too large

	- Max Dimension Size on GT 750M: 8192

	- Max Dimension Size on Mesa DRI Intel(R) Haswell Mobile: 8192

* Initial FPS: N/A

* Shader Texture Texels: 8,388,608

* Main Texture Texels: 67,108,864

* Lightmap Texels: N/A

* Total Texels: 75,497,472

* Total Bytes: 301,989,888

#### Branch: atlas_improvements (new generation method)

**Map: Railgun_Arena.bsp**
_____________________________________

* Initial FPS: ~ [52, 55]

* Shader Texture Texels: 4,194,304

* Main Texture Texels: 2,097,152

* Lightmap Texels: 2,097,152

* Total Texels: 8,388,608

* Total Bytes: 33,554,432

**Map: q3tourney2.bsp**
_____________________________________

* Initial FPS: ~ [73, 88]

* Shader Texture Texels: 2,097,152

* Main Texture Texels: 8,388,608

* Lightmap Texels: 524,288

* Total Texels: 11,010,048

* Total Bytes: 44,040,192
__________________________________

**3/18/16**

Just tested q3dm1.bsp, and...crash! Atlas generation hits over 16K in height, so it looks like

there will need to be some other spacial optimizing modifications made as well. A few to start out with:

- Pass in the max dimension size which is queried from GL; if either the width or the height hits this value during generation,
add another column or row as a means to shorten the offending dimension.

- Check to see if the second-to-largest dimension is half or less than the largest. If this this the case, even things out
by throwing the largest dimension category in another column or row.

These should help.

**3/19/16**

Column splitting at a very basic level is implemented. There needs to be some

actual statistics used in determining when a column should be split, though. Incidentally,

it appears that some of the images aren't in their proper order within the column...this

is definitely odd. To make matters worse, there's a large amount of space between

the columns when the offset is applied to width slots that hold more than one column;

this will need to be corrected if it's to be used properly.

TODO:

	- ~~Fix the spacing issues; there shouldn't be a gap between the image widths.~~

	- ~~Verify the ordering of the buckets - does the ordering get fucked up *after* the
	column is split, for example?~~ (**update** ordering really isn't screwed up)

- ~~Add the statistics; one possible heuristic for determining when a width's column should
	be split is when it's 2 or more deviations from the bucket count of the width
	slot that is second highest to its own bucket count
	(without aggregating bucket counts for multiple columns which belong to a single
		width - the counts should be evaluated separately).~~

	- ~~Look more into images which involve dimensions that aren't powers of two, and~~
	then figure out how to deal with them. (**update** there really isn't a need to worry
	about this, considering that any NPOT images will be stored in a POT texture atlas
	anyway).


**3/20/16**

The gaps are closed. Added the necessary statistics computations (zed-score, standard deviation, average, etc.)

to get things up and running. The "nextCol" in the zHigh conditional isn't necessarily

part of the algol, but it's there to see what happens when a bucket is "split" into two separate

buckets (as opposed to just taking one bucket from a column and turning it into a new column).

The bucket split maneuver doesn't quite work, because in TraverseColumn() there's an evaluation

for ReadOffset() which checks to see if its value is > 1 before a y offset

for that height is applied. Hypothesis: since the new ReadOffset value

is initially half of what it would have been before the split, only half of the images

of the original column are added...in a sense. What's really happening is that half of the images

are being added, and then the next half are being painted over them. This seems to be because,

after the first half of images are added, there's no reason to continue onto the next

column, considering that curr is != null ( which is what TraverseColumn's return value is

parallel to).

So, that's next on the TODO list: fix this bucket split issue.

**3/22/16**

Bucket splitting is taken care of. Going to work on grid splitting next, as a mechansim for

working with generated atlasses which surpass GL_MAX_TEXTURE_SIZE in the width and/or height.

**3/23/16**

The good is that we have the following taken care of:

* Grids are partitioned, and are capable of generating textures without killing the GL.

* Grids are searchable.

The bad news:

* Shit looks fucked up for grids with quantities > 1. Basically, undefined behavior.

So, what to do? After writing overlap tests and resolution methods (to ensure that
images which are in certain locations are adjust so that there's no overlap
between two grids, while at the same time preventing overlap with neighboring
images), it seems like the initial locations which are being searched may be somewhat off,
considering that the resolution/collision routine doesn't seem doing much
with the current data set.

So, the positioning and initial locations need to be investigated further. Definitely
continuing to test the atlasses in TTextureTest will be useful; it would be handy
to implement a mechanism which allows one to view the images on a grid-by-grid basis.

**3/24/16**

Grid issue is fixed. Added support for viewing individual
grids in the texture unit test. Also fixed an issue which required disabling depth writes
for transparent surfaces (unless otherwise specified).

**3/25/16**

#### todo

- ~~Get Emscripten build re-up and running. (in progress):~~

Rewrote a lot of the post-generation functionality to append the relevant
path files directly in the source. So far, so good. Still need to actually
test the asset loading and make sure everything is good, though.

That's next on the list.

- Optimize program data uploads ( see *Shader Programs* entry for **3/7**)
- Work on sky effects

**3/28/16**





The browser requires a certain amount of memory to load the asset files.
Currently, the upper bound I'm using is 1 GB for the total memory which is allocated
by Emscripten. So, I'm working towards getting that down as much as possible.

I've created a web worker which should help with this: apparently, mounting
the WORKERFS instead of the MEMFS in the virtual filesystem allows for one to
take a more "piecemeal" approach to memory allocation as far as disk data is concerned.

At the moment, though, an exception is thrown on init, because the filesystem attempts
to find /tmp, which for some reason fails.

It could be that I need to specify the packages in WORKERFS manually, using the opt
argument.
The example, [here](https://kripken.github.io/emscripten-site/docs/api_reference/Filesystem-API.html#FS.mount), should provide a good starting point.

**3/29/16**

A much more in-depth issue appears to be popping up with the asset loading. The file
packages each define their own routines using Ajax requests which rely on functions from
the Module object. The problem is that these functions are defined when the actual program
.js file (bspviewer.js, in this case) is run. The intention here seems to be that the
async calls themselves will be finished after the functions have been defined.

Unfortunately, this isn't always the case. In fact, on the current system I'm testing on,
this never seems to be the case.

The catch-22 is that the bundle JavaScript files need to be executed before the main
file is loaded, given that the main file relies on asset data which is only accessible
through the file packages. So, the idea is that, by queuing up these requests, everything
should be resolved in time.

Some interesting insight is provided [here](https://github.com/kripken/emscripten/issues/1992).

The run() function is defined in postamble.js, on line 174. There  may be worthy
information in preamble as well.

**3/30/16**

A hack in shell.html gets around the removeRunDependency issue.
However, when running only the IO Worker test, I've noticed that there are some exceptions
which are thrown when there's an attempt by emscripten to mount a number of different
folders like '/home', '/tmp', etc. I have a feeling this is normal, but nonetheless,
I should test this while the renderer is running to make sure the same thing isn't happening.

Additionally, one of the emscripten devs on the IRC noted that the premature removeRunDependency
call sounded like a legitimate bug, so I should come up with a test case when time allows...

**3/31/16**

You still need to find a good workaround for defining Module.walkFileDirectory
so that it can be referenced by other inline JS snippets.

Afterward, click "pause on exceptions" in the debugger and see if there's problems
with it mounting the file system. According to Emscripten, as long as the app is being
served via Node, mounting NODEFS will provide access to the local filesystem, so these
attempts to read from different directories could actually be due to that.

**4/1/16**

Ran some tests after finding a means to get Module.walkFileDirectory defined correctly.

The renderer's up and running, and it turns out that most of hte performance hit
was due to the insane amount of IO which was occurring throughout every frame.

After testing debug and release, it seems like the majority of the time spent per frame
is taken up by the bounds tests for the view frustum. There's still a lot to figure out,
here, but this is at least a start.

Either way, the original exceptions being thrown by the file system were indeed
NODEFS related, and were happening because NODEFS wasn't mounted - it likely won't be
used for this project.

Now what's left to do is to just use worker threads to get the bundles properly loaded
in a way so they don't take up nearly as much memory.

#### todo
- Find a more efficient means of loading asset data.
- Optimize program data uploads ( see *Shader Programs* entry for **3/7**)
- Work on sky effects

**4/2/16**

- worker thread functions should be exported (may want to double check the syntax
	in gen_workers.sh)
- getting memory error, which could either be due to a) the logging of the bsp data, b)
the amount of copying required in the worker thread and its communication with the
main thread, or c) both.

- appears that file read is successful, considering that exception is thrown at the end
of Q3BspMap::ReadFile for LogBspData. However, the following is worth noting:
  	- There is no return false in  File_GetBuf
	- Not seeing any indication that XHR is actually being loaded, given that there's
	no output to the screen as there should be.

So, it probably isn't successful. Also, try recommenting in ghetto await that loop, seeing as
how it may still work.

**4/3/16**

- main problem is figuring out why Emtrepreter.Async.handle or whatever is failing
on assertion.
	- based on the error message, it seems like something is undefined.

	- minification is preventing you from being able to step through before the error
	actually happens; this is because there's just too much damn memory in that js file.

	- should compile/link with -O0 to avoid the minification bullshit that's
	preventing you from stepping through.

- Emterpreter is needed for emscripten_sleep_with_yield; you want this so you can
mount the bundles in a worker and wait until they're finished before continuing
execution on the main "thread"

**4/5/16**
Some things to keep note of:

	- JS workers don't have access to Module

	- The only way to simulate any semblance of decent blocking is to embed inline
	JS into the C++ source (as shown in worker_t::Await()), unless it's actually possible
	for the Emterpreter to get up and running (doubtful).

	- ReadFile worker function appears to work; however, there are exceptions being thrown
	when the Traversal worker function is used afterward. Traversal is used during effect
	shader parsing...

**4/13/16**

* It looks like it's possible to achieve the async asset loading in parallel

* New calling model is necessary: incorporate async callbacks; the renderer
shouldn't be initialized until all assets are loaded. This initialization will
specifically have to come from a callback function which is passed to the worker_t::Await (name
	should be changed) method.

* Test the actual async IO before rendering anything; load up a unit test for this. Make sure
the callback function is fired, and that all assets can be read from. Once that's in place,
all will be good.

**4/15/16**

Script is executing as it should be in the worker thread. However, you need to figure out
where the undefined error is occurring; could be just a reference to an undeclared
variable or something.

**4/15/16**

Threw the JavaScript in a separate source file, so it can be injected manually.

If the blob's data is going to be parsed manually, it can be converted to an ArrayBuffer
like so:

```
var arrayBuffer;
var fileReader = new FileReader();
fileReader.onload = function() {
    arrayBuffer = this.result;
};
fileReader.readAsArrayBuffer(blob);
```
[Source](http://stackoverflow.com/a/15981017/763053)

However, it's questionable whether or not this is a good route to take, because you
don't understand the whole underlying mechanism. The idiomatic way to achieve this
is still somewhat "cloudy".

Here are some related questions worth finding answers to:

* How is the WORKERFS expected to be accessed? Is fopen in a worker thread
actually supposed to work with WORKERFS, or is that limited to NODEFS and/or MEMFS?

* The cube example appears to use some kind of mechanism involving continuously
pushing fetched blob data to an array that's tied to the Module object; is this a viable
route for what you're trying to do?

* Would it be realistic to consider storing the bundles in memory, without using WORKERFS,
and copying data from them manually? In fact, could it be that this is the primary
*intention* behind WORKERFS to be used?

	- If this is the case, you'll want to load all of the packages in a global buffer,
	and slice the appropriate copy of the binary blob from them using the metadata JSON.

	- First thing is to just iterate over each metadata object and assess whether or not
	the path is referenced in it. Once you find the correct reference, you can just
	slice out a portion of memory from the corresponding blob. Note that this is going
	to be slow, so once you get this working you *absolutely* must make this more
	intelligent. For example, we know that the the blob and metadata bundle pairs each
	refer to a specific BSP data folder; this can be extracted from the desired path
	and used as a means of determining the bundle pair which holds the necessary file data.

From reading library_workerfs.js in emscripten's source, it seems like all of the needed
data for the packages option is provided. Yet, calling fopen using both bundle paths
(e.g., bundles/maps.data, /bundles/maps.data, /maps.data, maps.data, etc.) as well as
actual asset paths (e.g., /asset/stockmaps/maps/q3dm2.bsp) returns null file pointers.

You know, though, that the data is being properly fetched from the server's URL: you've
dumped the results in the console and seen them yourself. Furthermore, every blob
which is fetched produces a legitimate size of the blob's data.

So, maybe the problem is that it's compressed? You should create another bundle
folder which holds strictly uncompressed BSP data, and try loading from that instead.

Once you have this working properly, it would be beneficial to look more into
decompressing the data.

**4/16/16**

Exploring the internal structure behind the file systems. It looks like the permissions
given for all of the mounted files are supposed to be 0777, so it's doubtful that
the inability to fopen them is related to permissions (which makes sense, considering
that it's all virtualized).

The reason why fopen doesn't work despite the filesystem being mounted could be
related to something which is missing in the worker thread, specifically. In other words,
it very well may be that the data being sent is perfectly valid, but the connection
between the initial mount and fopen's function just isn't there.

Remember, fopen() will call syscall5(), which in turn is sending the proper path data down
the pipe. FS.open() itself is what fails. It could be that FS.open isn't actually
linked properly to the internal node system in WORKERFS. If so, looking into the
stream_ops section of WORKERFS should provide some alternatives; however, you'll
have to perform the actual file read in JavaScript, for ReadFile_Proxy.

Also, make sure that the FS.createNode function (which is called from WORKERFS.createNode),
directly adds the newly created node to its own lookup store - if it does, that would
increase the chance that FS.open should be able to access directories which are mounted
to WORKERFS.

Also, mounting '/' as opposed to '/working' may provide some further insight...

So, the following is a TODO list. Don't make any judgements until you've completed
all of the items.

- Make sure that FS.createNode adds the created node to its own lookup system, and
is more than just a constructor.

- Dive into FS.open and use Module.printObj to analyze what you can.

- Try mounting '/' as opposed to '/working'; just because '/working' is created via
mkdir in the VFS does not necessarily imply that mount will succeed...

**4/17/16**

Ok. So, apparently the FS.lookup() call uses a node_ops method (i.e., a vtable for
an inner lookup corresponding to a different file system). The funny thing is that
this node_ops function calls into the memfs lookup variant. Yet, for some reason
there hasn't been any indication that the mount via the workerfs will create nodes
that actually bind to node_ops instances lying within the memfs module. In addition, memfs lookup
(*and* its workerfs counterpart) are both stubbed. Whether or not this is yet-to-implemented
func, or just stubbed for the sake of interface (assuming IDBFS and nodefs use it...),
remains to be seen.

The mystery continues...


*Update*

~~Well, I'm pretty certain this is it: there appears to be no individual hash for root folders
that contain only one subdirectory. In this case, while "asset/stockmaps" is hashed,
"asset" by itself isn't, because stockmaps is only the subdirectory mounted in the path.~~

^^^At first it seemed like this was true. It's not...

So, I think what's next on the agenda is examining the path which is being sent to
FS.lookupNode: does it contain any slashes? (The answer is no: the parts array,
	which is used in sending dir paths to FS.lookupNode, sends 'asset' as its first directory
	lookup).

lookupPath contains two print functions at the moment, which can be used to
compare the state of the table around the time of earlier IO operations and
when fopen attempts to read the bsp map file.

There shouldn't be a reason for ReadFile_Proxy to *not* be a part of the worker. This
is a pretty significant detail.

Here are the functions you've added print statements to, so far. Remove them when
you're done (and add to this list as necessary).

FS.mount
FS.open
FS.lookupPath
FS.lookupNode
FS.lookup
FS.hashAddNode
SYSCALL.syscall5

And the TODO list:

- Dump the IO of the current build to a text file; the web console won't
output everything. Keep investigating...

**4/19/16**

It works. The source of the problem was here, in $EMSCRIPTEN/src/library_ws.js (line 11):

```
var createdParents = {};
function ensureParent(path) {
  // return the parent node, creating subdirs as necessary
  var parts = path.split('/');
  var parent = root;
  for (var i = 0; i < parts.length-1; i++) {
	var curr = parts.slice(0, i+1).join('/');
	if (!createdParents[curr]) {
	  createdParents[curr] = WORKERFS.createNode(parent, curr, parts[i], WORKERFS.DIR_MODE, 0);
	}
	parent = createdParents[curr];
  }
  return parent;
}
function base(path) {
  var parts = path.split('/');
  return parts[parts.length-1];
}
// We also accept FileList here, by using Array.prototype
Array.prototype.forEach.call(mount.opts["files"] || [], function(file) {
  WORKERFS.createNode(ensureParent(file.name), base(file.name), WORKERFS.FILE_MODE, 0, file, file.lastModifiedDate);
});
(mount.opts["blobs"] || []).forEach(function(obj) {
  WORKERFS.createNode(ensureParent(obj["name"]), base(obj["name"]), WORKERFS.FILE_MODE, 0, obj["data"]);
});
(mount.opts["packages"] || []).forEach(function(pack) {
  pack['metadata'].files.forEach(function(file) {
	var name = file.filename.substr(1); // remove initial slash
	WORKERFS.createNode(ensureParent(name), base(name), WORKERFS.FILE_MODE, 0, pack['blob'].slice(file.start, file.end));
  });
```

In particular:

```
var parts = path.split('/');
var parent = root;
for (var i = 0; i < parts.length-1; i++) {
  var curr = parts.slice(0, i+1).join('/');
  if (!createdParents[curr]) {
	createdParents[curr] = WORKERFS.createNode(parent, curr, WORKERFS.DIR_MODE, 0);
  }
  parent = createdParents[curr];
}
```

Each time `curr` is calculated, the name of the node is given the full absolute path,
as opposed to *just* the name of the folder itself that i represents. For example,
say we're adding the file 'asset/scripts/somescript.shader'; we'll have the following
parent nodes out of this:

```
"asset"
"asset/scripts"
```

And one child name of "somsecript.shader"

The thing is that any File System lookup that occurs through a call to FS.open has
the potential to rely on a call into FS.lookupPath to attain the information it
needs to continue. The FS lib (which WORKERFS implements as a
	"sub-interface") depends heavily on a hash table that's used to employ
	quick lookups in memory in the filesystem. If the item to be fetched isn't found
	in that table, then a default lookup function is called, which will lean towards
	a stubbed out MEMFS function (I haven't figured out why this is the case for
	a path with a non-existent hash value yet - it's weird, because the function
	which is stubbed is bound to a parent that can actually be part of the FS given
	that its 'curr' path is only a single name). In my experience all that's happened
	from this is a thrown exception (in the stub) which gets caught back in FS.open,
	and then ignored.

The fix was pretty simple, though.

This,

```
if (!createdParents[curr]) {
  createdParents[curr] = WORKERFS.createNode(parent, curr, WORKERFS.DIR_MODE, 0);
}
```

becomes this:

```
if (!createdParents[curr]) {
  createdParents[curr] = WORKERFS.createNode(parent, /*curr*/ parts[i], WORKERFS.DIR_MODE, 0);
}
```

And this is provable: we still use curr to maintain consistency and ensure the correct
parent is fetched in the table; since the newly created node is dependent
on its path within the parent table, which is in turn dependent on the path itself,
this doesn't break anything - it actually conforms to the standard the main FS module
expects.


**4/20/16**

Fixed another problem in Emscripten involving worker respond/request chains:
when a worker is called initially, and the callback supplied recalls
the worker with a different function, passing in different data than what
was passed before, if the new data has a size which is less than the first
payload, the underlying buffer won't be reallocated, and thus there
will be garbage output passed the buffer memory which exceeds the second
data's size.

(this occurs in postamble.js, and has been detailed more
	[here](https://github.com/kripken/emscripten/issues/4258))

Anyway, this was fixed because it's become clear that there needs to be
an arbitrary IO/chain mechanism in place for reading files: simply just
passing in an entire buffer of memory and then converting it to a struct
is error-prone, due to unspecified behavior that's implementation dependent
as far as C/C++ standards are concerned (a lot of this has to do with alignment).

So, the solution is to do consecutive reads, one by one. In order for this to work,
there has to be a chain mechanism setup, where the first function called
in the worker opens the file, notifies the callback that the file is ready to be read,
and then the callback invokes a different function in the same worker which
handles reads as necessary.

So, each BSP struct directory section will be read piece-meal.

Need to think about how exactly to go about this.

**4/21/16**

Running into a problem with reading the header file. the ID which is returned contains
the following char codes:
15,
90,
0,
2.

Out of these four, only one of these (90) is a letter: Z. This is what the actual
heap contains in the web worker thread, so there must be a problem with the file r
ead itself, in file_t::Read() (maybe the buffer reallocation? read alignment? Hmm...).


**4/21/16 (2)**

- Going to need to rewrite the process so that the Renderer isn't initialized
until the map is finished loading - since the Q3BspMap has an arbitrary
finishing function to call after it's done reading the memory, the initialization
of the renderer itself can be decoupled from the read. All data expected by
the renderer from the map should be therefore passed to the renderer explicitly
at this point. (this is pretty much done)

- Even though the map read event chain is properly laid out, the shader parsing
and reading still needs to be properly handled. This, too, should be taken care of
in the worker fs. So, you'll need to make sure walkFileDirectory() in fetch.js
has the right callback signature setup for itself. The model will be fairly simple:

```
(in worker thread)
for each file in dir:
 	if ext(file) === 'shader':
	 	read file into buffer;
		send provisional response with data
		(when the data is received on the other side, parse the entire shader
			file)

end worker (pass nullptr, which will signal that the traversal is finished to the response callback, which will then call its own "on finished" event function)
```

**4/22/16**

- After finishing each shader read, you want to refactor the shader generation
pipeline and its corresponding texture/sampler generation into its own
module which is separate from the effect shader parser. The renderer
shouldn't call S_LoadShaders; rather, it should call a function
which takes the Q3BspMap and uses its (already generated) effect shader data to
produce samplers/textures in an atlas, in addition to any relevant shader program
handles (REMEMBER: the map only shader check is important, and will need
	to occur before a shader is generated)

**4/22/16 (2)**

- Code path for loading texture images is mapped out. It's getting clear that
attempting to support both desktop and browser is becoming pretty much pointless
for this project, so the desktop support will likely be phased out in the process.

- The code path for async image loading is only setup for the first tier of images (
 those which correspond to the effect shaders directly ); the second tier, which
 comprises of the shaders/textures lump (still not sure what the original intention
 here is by the developers), still needs to be taken care of.

- The first tier needs to be tested. However, the ReadImageFile function still needs
to be implemented. Don't forget to prepend 8 bytes of metadata to the image buffer.
This is the format:

[0, 1] -> width
[2, 3] -> height
[4] -> bpp
[5, 7] -> padding

It may be beneficial to align the image data buffer if it isn't already as well;
it's been some time since you've read about caching, so maybe read up on it some more or
something.

**4/25/16**

- Loading the non-shader images leads to a crash (i.e., exception thrown). It looks
like a bad memory access, coming from an index which exceeds what's available in the
8-bit emscripten heap.

(also, note that the paths being passed likely don't have any extension specified
- there is a fail safe check for this in the image io module, but for some
reason the desired message isn't output, so it's questionable whether or not the
condition which raises that error in particular is caught when the exception is
thrown)

**4/26/16**

Fixed last issue: the height/width check values in OnReadImage in the image
async module were being fetched as integers, which blew things out of proportion.

The "there is no file extension - weird" message is displaying, now. It's good
that it doesn't actually kill the process, because it's left open another
error which is happening in `_asm.free`:


```
$71 = ($45|0)==(0|0);
   if ($71) {
    $p$1 = $15;$psize$1 = $16;
   } else {
    $72 = ((($15)) + 28|0);
    $73 = HEAP32[$72>>2]|0;
    $74 = (7768 + ($73<<2)|0);
    $75 = HEAP32[$74>>2]|0;
    $76 = ($15|0)==($75|0);
    if ($76) {
     HEAP32[$74>>2] = $R$3;
     $cond20 = ($R$3|0)==(0|0);
     if ($cond20) {
      $77 = 1 << $73;
      $78 = $77 ^ -1;
      $79 = HEAP32[(7468)>>2]|0;
      $80 = $79 & $78;
      HEAP32[(7468)>>2] = $80;
      $p$1 = $15;$psize$1 = $16;
      break;
     }
    } else {
     $81 = HEAP32[(7480)>>2]|0;
     $82 = ($45>>>0)<($81>>>0);
     if ($82) {
      _abort();
      // unreachable;
     }
```

$82 evaluates to true. The cause of it, though, is odd. Following the code
path in the function up to this point, it's clear that $15 is a pretty
significant player. Its value appears to represent a memory address
which is computed by some value that's relevant to what memory
is located 8 bytes behind the pointer to be freed. There's lots of comparisons
between $15 and other memory locations which have been computed. What's not clear
is these values, and the meaning behind the constant addresses. Examples of this
include meaningful values attained from memory locations which are multiples of
4 after $15. Addresses $15 + 8, $15 + 12, $15 + 16, $15 + 24, and $15 + 28 are
evaluated (and possibly more).

$15's value is computed as follows (in a nutshell):

```
$1 = $mem - 8
$12 = HEAP32[$1 >> 2]
$15 = ($1 - $12)
 ```

 So, whatever lies at $1 represents an apparent boundry. The offset computations mentioned
 above might be some reference counting mechanism: there are situations where
 memory locations derived from one of the offsets will refer to a value
 that's equivalent to $15. This is just speculation, though.

 The best thing to do would be to ask the folks at Emscripten about this. That is,
 unless this function (before compilation) exists somewhere in the Emscripten src folder,
 which it may very well not...

**6/10/16**

Decided to take a look at it from a higher level. So far, it looks like that, while
the second-to-last image path fails, the OnImageRead function isn't called at all, 
so it's not possible to recover via a fallback extension - this is also weird.

So, what needs to be figured out is why OnImageRead isn't being called (the emscripten_worker_respond()
function is called even for failures, so this shouldn't be it...

**6/11/16**

The problem is that the actual fopen call in the web worker function (via gFIOChain)
doesn't have any idea of a fallback extension mechanism incorporated for file paths which it can't
initially find. So, this will need to be passed as part of the stream of data.

The next step is to map a binary search into the find mechanism for the file format, 
considering that it will actually prove much more useful that way. Each index of i "points"
to an inner array of elements which will be 2^i in length. 

**Update**

Added a quick fix to see whether or not the issue was solely based on lack of falling back to jpeg. 
It did indeed help, quite a bit. There's _still_ some major free issues going on, though.

The free function fails at a comparison between two addresses; if they aren't equal, then an 
abort() is called. What's weird is that these two addresses are like a gigabyte apart from
each other. To make things more interesting, the higher addresses are actually odd and not
four byte aligned. So, it seems like the design is setup so that there's a circular
reference going on and that circular reference isn't being properly hit due to 
some kind of memory corruption. The circular reference involves
the base of the pointer referring to a specific address (at 8 bytes from
its base). This address it refers to in turn has an offset which
points back to the base of the pointer...

It's also worth looking at the logic from the beginning of the free call which _leads_ up that
point though, too.

What's also worth mentioning is that there appear to be situations
where a given filepath will fail on being located via fopen,
but will, for some reason, immediately retry that exact same
path as many as 3 times. This doesn't make sense, given
the 3 consecutive checks that potentially occur, and the fact that
each of those checks either changes the path extension or releases
the worker thread from working.

It makes me wonder if resources are being unnecessarily duplicated...
