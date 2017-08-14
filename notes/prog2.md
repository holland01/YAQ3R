### 5/4/2017

* ~~make sure that the optimized build doesn't crash~~

~~* Try the frustum idea and see if it helps with the framerate when the camera flies out in the open. Make sure you compare FPS before and after; maybe use a flag that computes
the extra paralell planes if true and doesn't if false.~~

* Forget about the Frustum for now, see about leveraging timing via input and desired
framerate using the camera update method and tracking the desired FPS.

### 5/13/2017

* Figure out source of emscripten compilation issue: you need makeThrowVerbose to work so you can diagnose what's causing
the renderer to freeze when q3dm13.bsp is loaded.

### 5/14/2017

* Despite modifications to parseTools.js and library.js the thrown exception doesn't seem to take into account the extra arguments passed. Figure out
why...

### 5/20/2017

crash happens in DrawMapPass, with a non-zero texture index
called into BindTexture. Here is the offending call:
(the heap16 buffer is minified, represented as 'b').



__ZN11BSPRenderer11BindTextureERK7ProgramRKNSt3__210unique_ptrIN3gla7atlas_tENS3_14default_deleteIS6_EEEEtPKci(0, j, i, b[(__ZNKSt3__213unordered_mapIjtNS_4hashIjEENS_8equal_toIjEENS_9allocatorINS_4pairIKjtEEEEE2atERS7_(m + 128 | 0, h) | 0) >> 1] | 0, 29543, 0)

entire minified function:

(note that 'a' variable is the this pointer)

```
function __ZN11BSPRenderer11DrawMapPassEiiNSt3__28functionIFvRK7ProgramEEE(a, d, e, f) {
       a = a | 0;
       d = d | 0;
       e = e | 0;
       f = f | 0;
       var g = 0
         , h = 0
         , i = 0
         , j = 0
         , k = 0
         , m = 0;
       g = l;
       l = l + 16 | 0;
       h = g;
       Xf(515);
       Vd(1, 0);
       c[h >> 2] = 0;
       c[h + 4 >> 2] = 0;
       c[h + 8 >> 2] = 0;
       __ZNSt3__212basic_stringIcNS_11char_traitsIcEENS_9allocatorIcEEE6__initEPKcj(h, 29410, 4);
       i = __ZNSt3__212__hash_tableINS_17__hash_value_typeINS_12basic_stringIcNS_11char_traitsIcEENS_9allocatorIcEEEENS_10unique_ptrI7ProgramNS_14default_deleteIS9_EEEEEENS_22__unordered_map_hasherIS7_SD_NS_4hashIS7_EELb1EEENS_21__unordered_map_equalIS7_SD_NS_8equal_toIS7_EELb1EEENS5_ISD_EEE4findIS7_EENS_15__hash_iteratorIPNS_11__hash_nodeISD_PvEEEERKT_(a + 36 | 0, h) | 0;
       if (!i) {
           j = Yf(8) | 0;
           __ZNSt11logic_errorC2EPKc(j, 39974);
           c[j >> 2] = 27356;
           _b(j | 0, 2560, 392)
       }
       j = c[i + 20 >> 2] | 0;
       __ZNSt3__212basic_stringIcNS_11char_traitsIcEENS_9allocatorIcEEED2Ev(h);
       __ZNK7Program25LoadDefaultAttribProfilesEv(j);
       if ((d | 0) < 0) {
           i = a + 8 | 0;
           __ZN11BSPRenderer11BindTextureERK7ProgramRKNSt3__210unique_ptrIN3gla7atlas_tENS3_14default_deleteIS6_EEEEtPKci(0, j, i, (c[c[i >> 2] >> 2] | 0) + 65535 & 65535, 29543, 0);
           k = i
       } else {
           i = a + 4 | 0;
           m = c[i >> 2] | 0;
           c[h >> 2] = d;
           __ZN11BSPRenderer11BindTextureERK7ProgramRKNSt3__210unique_ptrIN3gla7atlas_tENS3_14default_deleteIS6_EEEEtPKci(0, j, i, b[(__ZNKSt3__213unordered_mapIjtNS_4hashIjEENS_8equal_toIjEENS_9allocatorINS_4pairIKjtEEEEE2atERS7_(m + 128 | 0, h) | 0) >> 1] | 0, 29543, 0);
           k = a + 8 | 0
       }
       if ((e | 0) < 0)
           __ZN11BSPRenderer11BindTextureERK7ProgramRKNSt3__210unique_ptrIN3gla7atlas_tENS3_14default_deleteIS6_EEEEtPKci(0, j, k, (c[c[a + 8 >> 2] >> 2] | 0) + 65535 & 65535, 39960, 1);
       else
           __ZN11BSPRenderer11BindTextureERK7ProgramRKNSt3__210unique_ptrIN3gla7atlas_tENS3_14default_deleteIS6_EEEEtPKci(0, j, k, e & 65535, 39960, 1);
       c[h >> 2] = 0;
       c[h + 4 >> 2] = 0;
       c[h + 8 >> 2] = 0;
       __ZNSt3__212basic_stringIcNS_11char_traitsIcEENS_9allocatorIcEEE6__initEPKcj(h, 35540, 11);
       __ZNK7Program8LoadMat4ERKNSt3__212basic_stringIcNS0_11char_traitsIcEENS0_9allocatorIcEEEERKN3glm7tmat4x4IfLNS9_9precisionE0EEE(j, h, (c[a + 120 >> 2] | 0) + 72 | 0);
       __ZNSt3__212basic_stringIcNS_11char_traitsIcEENS_9allocatorIcEEED2Ev(h);
       __ZNK7Program4BindEv(j);
       h = c[f + 16 >> 2] | 0;
       if (!h) {
           f = Yf(4) | 0;
           c[f >> 2] = 3308;
           _b(f | 0, 168, 48)
       } else {
           kh[c[(c[h >> 2] | 0) + 24 >> 2] & 255](h, j);
           __ZNK7Program7ReleaseEv(j);
           Pa(33984);
           ad(3553, 0);
           Pa(33985);
           ad(3553, 0);
           l = g;
           return
       }
   }
```
### 7/21/2017

It's been 2 months since I've touched this; thankfully I'm pretty sure I've figured
out the issue. Here's the thing: invalid image paths might not be inserted
into the key map. That's fine. BUT, that doesn't change the fact that
bspFace_t entries are going to have indices into the bspShader_t buffer
and in turn will be used to access image info from an external atlas key map.

So, with two images removed there's an out of bounds access which occurs
and causes the crash. The index is 99, so apparently that would be
the flareShader entry.

What's left to do is finish up the Mark/Sweep deal so that once the map load has finished
the list of bad indices can be iterated over so the map data is modified as needed.

### 7/23/2017

There are going to be shaders without valid filepaths; these are still important to
keep, though, because the bspFace_t list will likely contain faces which refer
to them via the shader/texture index being lsited. For some reason one shader
in particular is elusive in the sense that it's never even detected by the
async_io image loader...WTF???

### 7/24/2017

There's some helper functions in debug_util.js which will help you get
the values of BSPRenderer::map and Q3BspMap::name. What you want is to write
another helper function which will give you the name of a bspShader_t entry
given an index. In this case, you want to look up entry 68, as the name
for that shader should provide some insight as to what is going on...

### 7/25/2017


**First Bit**


IMPORTANT: binary layout for basic_string is the definition used when
_LIBCPP_ABI_ALTERNATE_STRING_LAYOUT IS defined

(at least on Linux - this may or may not vary between platforms)

```
struct __long
{
    pointer   __data_;
    size_type __size_;
    size_type __cap_;
};

struct __short
{
    value_type __data_[__min_cap];
    struct
        : __padding<value_type>
    {
        unsigned char __size_;
    };
};

struct __raw
{
    size_type __words[__n_words];
};

struct __rep
{
    union
    {
        __long  __l;
        __short __s;
        __raw   __r;
    };
};
```

"textures/gothic_door/tim_dmarch01"

**Second Bit**

The data for q3dm13 is large enough to where the SEGMENT_SIZE variable
in fetch.js:AL.addSliceMeta is less than the blob's data. When this was
originally written, an outline was created which handled the data accordingly,
but didn't finish the necessary lifting required for fetching remaining slices
necessary.

You'll need to modify this fetch.js code (and probably some code in other
areas, including file_traverse.cxx) so that it continuously loads the next
slice as needed. This will involve a back-and-forth like mechanism
between LoadAndStreamImages() and AL.loadFinished - the code in async_image_io
shouldn't have to worry about this at all, unless the iterator information
it has is inconsistent, which is possible (compare the termination mechanism
being used).

Starting on line 358 in fetch.js is this:

```
// Here we're done: we only care about requested files that exist in
// the bundle at this point. If we find that enough images
// supersede our memory constraints then the implementation
// will be altered.
metadata.files = null;
```

You want to comment out this line, given that the comment above insinuates that
this was pretty much during the time when slices weren't necessary. They
are now, though.

You could also ditch this entire content pipeline deal and throw together
a tool which packages all data necessary for one single map into a binary
file. This would eliminate load time issues altogether, because a smaller
image can just be preloaded into the binary as was the case originally.

### 7/26/17

1) Make sure slice offsets are kept track of.

NOTE: there's a crash which happens in the compiled version of LoadImagesBegin.
Looks like there's an issue involving the gPathMap_t vector. It could
be inlining AIIO_ReadImages, and there's areas where the vector is
passed to a constructor and hence possibly moved (it's not a reference pass).

If so, there's code right after which uses the now-empty vector variable and
that could be causing issues. It's been replaced with the image tracker's buffer,
so just test it and make sure it works.

One last thing: the issue seems to be similar to what was happening before,
so move semantics may also be playing a role here, outside of just the
amount of data obtained.

### 7/28/17

Somehow a bad bundle path is getting passed via GU_LoadShaderTextures

bspviewer.html:1237
bspviewer.html:1237 [ GU_LoadShaderTextures | INFO ]: Called
file_traverse.js:171 ALL = |
file_traverse.js:171 BUNDLE = ; PATHS =
VM26:72 TYPEOF STR =  number ; STRING VALUE =  5267280 ; ret =  
VM26:72 TYPEOF STR =  number ; STRING VALUE =  5267281 ; ret =  
VM26:72 TYPEOF STR =  number ; STRING VALUE =  10288 ; ret =  6931
worker/file_traverse.js:171 ALL = |
worker/file_traverse.js:171 BUNDLE = ; PATHS =
3(unknown) TYPEOF STR =
bundle/.data Failed to load resource: the server responded with a status of 404 (File not found: /home/amsterdam/Devel/proj/yaq3r/bundle/.data)
bundle/.js.metadata Failed to load resource: the server responded with a status of 404 (File not found: /home/amsterdam/Devel/proj/yaq3r/bundle/.js.metadata)
VM26:430 THE PATH =  
(unknown) THE PATH =
VM26:329 Uncaught Expected a pipe delimited list of file paths for the bundle
worker/file_traverse.js:329 Uncaught Expected a pipe delimited list of file paths for the bundle


NOTES
====

* There shouldn't be any issues with sending the files provisionally: it looks
like the event queue works itself out here and prevents race conditions, so
things remain synchronous.

* Don't forget that if one image file isn't found for a given bundle
that bundle will get sacked; consider just sending provisional NULL data
so the client can adjust the data for the renderer accordingly

* The "paths" which are passed to the loader aren't being used; you don't
want to load the entire blob, but only mere slices of the blob corresponding to the
files you wanted to send. That's why the setup was working in the manner it was.
The fetch.js file is reverted so this should be taken care of now.

### 7/30/17

This is bad:

```
bool atEnd =
    ( size_t ) gImageTracker->iterator >=
        gImageTracker->textureInfo.size() - 1;
```

The server will iterate through each filename matched in the bundle's data,
and if any of the files iterated turn out to be unusable, a nullptr is sent
to indicate failure.

Furthermore, the final message is always sent after every file has been iterated
over, so definitely is an N + 1 sequence here.

That said, there is always the chance that some of the requested files aren't
found in the  bundle loader's serialization code path (during the initial
"slice" creation).

One requested file not being available would explain why `atEnd` wasn't true
during the final call to `OnImageRead()`; `AssignIndex()` is what increments
the iterator, and it's called irrespective of whether or not there is a
failure, but it's not supposed to be called during `OnImageRead()`'s ending
final call: it should at that point be equal to the size of `textureInfo`.

So, the only realistic explanation for why `atEnd` didn't resolve to true when
it should have is that there's a mismatch between `textureInfo.size()`
and `gBundle->GetNumFiles()`.

A method has to be devised to ensure that there is proper synchronization;
if any of the requested files aren't found. You can compare `undefCounter` with
the length of the generated `files` list in `AL.addSliceMeta()` to
determine this: if they're not equal then obviously there's a hiccup.

That said, this is also going to affect the indexing used to generate the
texture atlasses, since they depend on `gImageTracker->iterator`.

* Fix heap corruption

* Do an `MLOG_INFO` purge

* See if you can render without a crash.

### 8/3/17

There's a .tga texture file used in a shader stage
that should be producing a textureIndex != -1 for the stage
after the map data has been read, but isn't.

The last stage in the following dump is the offender.

Need to figure out why this is happening.

```
[ DrawEffectPass | INFO ]: texIndex: -1
bspviewer.html:1237
bspviewer.html:1237  shaderInfo_t{
bspviewer.html:1237 	deform: 0,
bspviewer.html:1237 	deformCmd: 255,
bspviewer.html:1237 	deformFn: 255,
bspviewer.html:1237 	deformParms: <stubbed out>,
bspviewer.html:1237 	cullFace: 0,
bspviewer.html:1237 	surfaceParms: 8568832,
bspviewer.html:1237 	localLoadFlags: 0,
bspviewer.html:1237 	tessSize: 0,
bspviewer.html:1237 	stageCount: 3,
bspviewer.html:1237 	surfaceLight: 0,
bspviewer.html:1237 	&name[ 0 ]: textures/sfx/flame1side,
bspviewer.html:1237 	stageBuffer: <stubbed out>,
bspviewer.html:1237 }
bspviewer.html:1237
bspviewer.html:1237 -------------------
bspviewer.html:1237 [0]
bspviewer.html:1237 shaderStage_t{
bspviewer.html:1237 	depthPass: 0,
bspviewer.html:1237 	textureIndex: -1,
bspviewer.html:1237 	tcgen: 0,
bspviewer.html:1237 	blendSrc: 1,
bspviewer.html:1237 	blendDest: 1,
bspviewer.html:1237 	depthFunc: 515,
bspviewer.html:1237 	rgbGen: 255,
bspviewer.html:1237 	alphaFunc: 0,
bspviewer.html:1237 	mapCmd: 0,
bspviewer.html:1237 	mapType: 0,
bspviewer.html:1237 	effects: <stubbed out>,
bspviewer.html:1237 	alphaGen: 0,
bspviewer.html:1237 	&texturePath[ 0 ]: ,
bspviewer.html:1237 }
bspviewer.html:1237
bspviewer.html:1237 -------------------
bspviewer.html:1237 [1]
bspviewer.html:1237 shaderStage_t{
bspviewer.html:1237 	depthPass: 0,
bspviewer.html:1237 	textureIndex: -1,
bspviewer.html:1237 	tcgen: 0,
bspviewer.html:1237 	blendSrc: 1,
bspviewer.html:1237 	blendDest: 1,
bspviewer.html:1237 	depthFunc: 515,
bspviewer.html:1237 	rgbGen: 255,
bspviewer.html:1237 	alphaFunc: 0,
bspviewer.html:1237 	mapCmd: 0,
bspviewer.html:1237 	mapType: 0,
bspviewer.html:1237 	effects: <stubbed out>,
bspviewer.html:1237 	alphaGen: 0,
bspviewer.html:1237 	&texturePath[ 0 ]: ,
bspviewer.html:1237 }
bspviewer.html:1237
bspviewer.html:1237 -------------------
bspviewer.html:1237 [2]
bspviewer.html:1237 shaderStage_t{
bspviewer.html:1237 	depthPass: 0,
bspviewer.html:1237 	textureIndex: -1,
bspviewer.html:1237 	tcgen: 0,
bspviewer.html:1237 	blendSrc: 1,
bspviewer.html:1237 	blendDest: 1,
bspviewer.html:1237 	depthFunc: 515,
bspviewer.html:1237 	rgbGen: 255,
bspviewer.html:1237 	alphaFunc: 0,
bspviewer.html:1237 	mapCmd: 2,
bspviewer.html:1237 	mapType: 1,
bspviewer.html:1237 	effects: <stubbed out>,
bspviewer.html:1237 	alphaGen: 0,
bspviewer.html:1237 	&texturePath[ 0 ]: textures/sfx/flameball.tga,
bspviewer.html:1237 }
```

Also, so far, this really seems to be the only file list (sent to fetch.js)
which actually has missing paths - that said, these are all "files" (really
just shader entry titles) acquired from GU_LoadMainTextures.
```
Array(88)
9
:
"/asset/textures/dummy_image"
10
:
"/asset/textures/common/caulk"
21
:
"/asset/textures/skies/tim_dm3_red"
34
:
"/asset/textures/common/clip"
35
:
"/asset/textures/organics/wire02_f2"
36
:
"/asset/textures/common/hint"
39
:
"/asset/textures/gothic_trim/pitted_rust2_trans"
45
:
"/asset/textures/gothic_light/ironcrosslt2_5000"
46
:
"/asset/textures/gothic_light/ironcrosslt2_3000"
48
:
"/asset/textures/gothic_block/killblock_i4b"
49
:
"/asset/textures/gothic_light/ironcrosslt2_10000"
50
:
"/asset/textures/gothic_light/pentagram_light1_5K"
54
:
"/asset/textures/liquids/lavahellflat_400"
55
:
"/asset/textures/common/nodraw"
56
:
"/asset/textures/sfx/flame1side"
60
:
"/asset/textures/gothic_light/ironcrosslt2_1000"
64
:
"/asset/textures/common/weapclip"
74
:
"/asset/textures/sfx/flame1dark"
79
:
"/asset/textures/common/donotenter"
81
:
"/asset/textures/gothic_block/killtrim_trans"
82
:
"/asset/textures/skin/skin5_trans"
83
:
"/asset/textures/skin/skin6_trans"
86
:
"/asset/textures/dummy_image"
87
:
"/asset/textures/common/trigger"
```

Entry 56 "flame1side" is the shader entry which uses the offending stage.

It might be that using GU_LoadMainTextures isn't necessary at all, and is
only causing issues. Maybe.

### 8/5/17

There's a default shader going on in the quake code that's designed to handle images
which aren't bound to a shader. So, it's going to be important to figure out how exactly
this is the case - i.e., are there texture paths in the shaders buffer and not just actual shader names?

What is the default shader index in the quake code, and where is the default shader actually defined?

Note that s_worldData.shaders is loaded strictly from the map file lump.

Also, remember that for thsi map no face->shader indices are out of bounds of the shader buffer.

### 8/5/17

There's a number of token identifiers in the shader files which vary between upper/lower case.
One token will often fluctuate between one or the other. This was initially attempted to be brute forced
by just lowering every token read from the shader, but that brought about other complications - notably,
the crashing issues and the odd inconsistences that have been happening.

At the moment, looks like things are on the right track. There's some really weird clipping issues happening, though.

The rgbgen entry in the effect shaders defaults to vertex coloring if the corresponding token isn't recognized; a better
default for this would be the "identity" option.

Your TODO is in this order:

* For now, really make sure there isn't any duplication going on between effectShaders and non-effect shader images.
If an effect shader refers to a given image in one of its stages, that very same texture should never be loaded in
the "noshader" image texture atlas.

* Cleanup the MLOG_INFO calls.

* Then, compile a release build. See if the clipping still happens and assess the performance. This might
be due to issues with the frustum culling or it could just be major problems with the GPU and CPU synchronization
given that the code is clearly executing slowly.

* Assuming no issues are happening, you want to test chrome/firefox on Windows, Mac, Linux.

### 8/6/17

**bugfix/any_wall_clip_weirdness**

Right, so the faces are ordered according to the following criteria:

type->lightmap->shader->drawSurface_t 

This is only efficient, though, if there's a significant amount of drawSurface_t's packed together
in a single bucket list. Otherwise, you're still going to get a large frequency of different texture bindings for both
lightmaps and normal images.

Furthermore, these aren't being depth sorted. The depth sort should obviously happen relative to the view-transform, for
both solids and transparent groups.

So, a new method needs to be put together.

For now, forget grouping according to attributes apart from transparency and depth. Get rid of the quadruple loop and
the crazy hashmap setup.

### 8/8/17

There might be some alpha channel issues with respect to SDL 2 and emscripten/webgl. It looks like SDL 2 doesn't provide
an alpha-based pixel format. Trying to explicitly set this in the generated asm.js doesn't change anything either. 
You need to research this more and see what the deal is. [This](https://stackoverflow.com/questions/35372291/alpha-rendering-difference-between-opengl-and-webgl) provides a little more insgiht, but it's not sufficient.

Anyway, as far as the view-depth sort, your best bet is to do the following:

  For each face:
    define a transformation using the S,T vectors which come with it; take the cross product to produce the Z-axis. Make sure that 
    the Z-axis direction is consistent with the front face culling method (should be CCW but only works with CW - might due to coordinate swizzling
    which happens in the beginnign, honestly not sure and will need to think ont hat because it could be important). 

### 8/9/17

Pretty sure I have this figured out right now. The key is to sort the faces/surfaces according to the sort parameter specified in their corresponding
shader. If the surface is transparent, the default is additive; if opaque, then "opaque" is the setting to fall back on. 

There's this in tr_local.h:

```
// any changes in surfaceType must be mirrored in rb_surfaceTable[]                                                                                          
typedef enum {                                                                                                                                               
    SF_BAD,
    SF_SKIP,                // ignore                                                                                                                        
    SF_FACE,                                                                                                                                                 
    SF_GRID,
    SF_TRIANGLES,                                                                                                                                            
    SF_POLY,                                                                                                                                                 
    SF_MD3,                                                                                                                                                  
    SF_MD4,
    SF_FLARE,
    SF_ENTITY,              // beams, rails, lightning, etc that can be determined by entity                                                                 
    SF_DISPLAY_LIST,                                                                                                                                         
    
    SF_NUM_SURFACE_TYPES,       
    SF_MAX = 0x7fffffff         // ensures that sizeof( surfaceType_t ) == sizeof( int )                                                                     
} surfaceType_t;                                                                                                                                             

typedef struct drawSurf_s {
    unsigned            sort;           // bit combination for fast compares                                                                                 
    surfaceType_t       *surface;       // any of surface*_t                                                                                                 
} drawSurf_t;                                                 
```

as well as this:

```
#define MAX_DRAWIMAGES      2048
#define MAX_LIGHTMAPS     256
#define MAX_SKINS       1024


#define MAX_DRAWSURFS     0x10000
#define DRAWSURF_MASK     (MAX_DRAWSURFS-1)

/*

the drawsurf sort data is packed into a single 32 bit value so it can be
compared quickly during the qsorting process

the bits are allocated as follows:

21 - 31 : sorted shader index
11 - 20 : entity index
2 - 6 : fog index
//2   : used to be clipped flag REMOVED - 03.21.00 rad
0 - 1 : dlightmap index

  TTimo - 1.32
17-31 : sorted shader index
7-16  : entity index
2-6   : fog index
0-1   : dlightmap index
*/
#define QSORT_SHADERNUM_SHIFT 17
#define QSORT_ENTITYNUM_SHIFT 7
#define QSORT_FOGNUM_SHIFT    2
```

the `sort` member in `drawSurf_t` relies on the bitmap specified above: a simple inequality comparison is used
between different sort values when the actual sort is performed - i.e., nothing fancy at all. 

Note also the `surfaceType_t`: it could very well be beneficial to make use of this in your implementation at some point.

What you want to do for now, though, is:

* define a simple default shader which uses the opaque setting for its sort parameter
* create two global shader lists (one for opaque, one for transparent) which store pointers to entries in the effectShaders map, and are both sorted according to their sort values (remember the defaults)
* make two lists for drawSurface_t*; remove the current hashmap craziness that's going on because it's not particularly useful. Maybe 
ditch it all together, since it looks like the performance improvement isn't really there for batched draws. None the less, the ordering
needs to be defined solely by the `sort` member. 
* make sure that the sort value for the drawSurface_t is properly set. For now, the only value needed to pay attention to is what's in bits 21-31: the sorted shader index (just use the bit layout for the original and not the TTimo modification to keep things simple)

On init you can presort all of the used shaders. 

For models, evaluating the z-depth is simple because a bounds is already defined for them. 

For faces, I'm still not sure what the best way to extract this would be. The world space st axes for the face could be used as described in the 
previous entry. But how do we know where the origin of these axes are? I suppose it's possible to cycle through all of the vertices within the face, 
look for min/max values, and then store those appropriately in a separate list. Using the st axes would be trivial from there.

Once this is all figured out, some major cleanup is in order... 

### 8/12/17

Still need to sort and draw the transparent faces. None the less, nothing's rendering. I know that "noshader" is a good fit for a default: it only contains one flag value, which is the first
bit in the contentsFlags member - CONTENTS_SOLID, implying map walls or something like that. So, this isn't the issue why hardly anything is showing. 

Adjusting the sort parameter (draw front to back, not back to front) seems like it could make a difference. The textures rendered are different, yes, but the issue of multiple faces being
tied to a constantly appearing/disappearing rectangle is still the case. 

So, some things to look investigate for tomorrow:

- shader uploads - what's happening? 

- Do we ever end up calling DrawFace with `PASS_DRAW_MAIN`?

- Is the sort index retrieved from the drawFace_t in `BSPRenderer::DrawFaceList` actually valid?

### 8/13/17

Figure out the issue with the last image path and then you're finished with bugfix/bad_effect_texture_indices.

Note that one image a relevant path refers to is read twice in a row, using the exact same shader stages (why is this happening?). 

There's two images that are relevant, and hence two paths. One appears to be read just fine, and 
the other is still missing. The image which is found ends with a .blend suffix, and uses the same name as the 
missing image file. 

The same list is being assigned two different indices. When initially constructed, there is no duplicate. My guess is that somehow
the path for the non blend variant is getting swapped with the blend variant (and likely its list in turn)