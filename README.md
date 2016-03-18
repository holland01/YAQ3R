bspviewer
=========

Pretty simple: just view a BSP map. This is really more of a project for educational/portfolio purposes than anything else.

It's a fun thing: despite Quake III's file format being written circa 1999, and therefore being extremely old, a lot of techniques with respect to computer graphics and optimization can be harnessed through a project like this.

Current Focuses:

- WebGL support
    * This is at the top of the list, because it's much easier to show something off via the web.
- Vertex deformation (effect shader)
- Texcoord mod functions (effect shader)

##Credits

### Source Code Read
___

* user **rohitnirma**l: https://github.com/rohitnirmal/q3-bsp-viewer?source=cc
* user **leezh**: https://github.com/leezh/bspviewer
* a programmer named **Paul**: http://www.paulsprojects.net/opengl/q3bsp/q3bsp.html

### Docs From teh Interwebz
___
* **Unofficial Quake 3 Map Specs**: http://www.mralligator.com/q3/
* **Rendering Quake 3 Maps**: http://graphics.cs.brown.edu/games/quake/quake3.html 

### Further
___
  * The original Quake III engine source code on Github.
  * Various other blog posts by the multitudes of others who have done this before I :)

### Minor Metrics

####Branch: emscripten (old texture atlas generation)

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

####Branch: atlas_improvements (new generation method)

**Map: Railgun_Arena.bsp**
_____________________________________

* Initial FPS: ~ [52, 55]

* Shader Texture Texels: 4,194,304

* Main Texture Texels: 2,097,152

* Lightmap Texels: 2,097,152

* Total Texels: 8,388,608

**Map: q3tourney2.bsp**
_____________________________________

* Initial FPS: ~ [73, 88]

* Shader Texture Texels: 2,097,152

* Main Texture Texels: 8,388,608

* Lightmap Texels: 524,288

* Total Texels: 11,010,048

* Total Bytes: 44,040,192
