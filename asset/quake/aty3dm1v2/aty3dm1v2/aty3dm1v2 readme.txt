Title:The Ancient Place  
Filenames: aty3dm1v2.pk3, aty3dm1 Readme 
File size: 18 Mb 
Author: Kaustic
Address: kaustic3@ptd.net.com

Description:Egyptian style map suited for fast gameplay.Mainly InstaUnlagged style of play.
         Althought this map is suitable for baseq3 style of play,the majority
         of the beta testing and play was done with the OSP and InstaUnlagged Mods. OSP config - Spawn 
         with all weapons,health and armor. No item/weapon pickups. Grappling hook enabled. Higher game speed
         and customized weapon damage and faster rail reload. No BFG.
=============================================================================================
Additional Credits:

   Bubba's Arena Tutorials - His website is where I learned enough to get started mapping.
   dONKEY - Mapping tutorials
   Cardigans Place - Tutorials on bot optimization of maps and mapping tutorials
   Bad Apple Mappery - Mapping tutorials
   Soc - For all of his cool textures and plants
   
   Quake3World Level Editing Forum - Thanks to the Moderators and all the people that
   give answers to those of us asking questions or looking for input.  
   
   Thanks to id for making this game in the first place.

   Everyone That used to hang out at map-center.com while it was still up and running.
=============================================================================================

Place aty3dm1v2 into your Quake3/baseq3 directory 


=============================================================================================
Information

In game Support - YES
SP BOT support - YES
Deathmatch - YES. 
New Textures - Yes  
Items/Weapons - Shotgun/Shells, Rocket Launcher/Rockets, Grenade Launcher/Grenades, Railgun/Slugs, Bullets, Health, Armor- Shards, Armor, Quad Damage, Haste, Med Kit
Regeneration.
=============================================================================================
Construction:

Base: New level from scratch.
Editor: Used:GtkRadiant-1.5.0  April 26,2007 
Utilities: ArenaMaster, Pakscape, Q3Map2Toolz,UI Enhanced v1.2
Known Bugs: None that I can find. :)


=============================================================================================

Thanks to the following people for all thier great work- (see readme's below)
=============================================================================


Xaero statue, by Oak

=============================================================================
Title                   	: Xaero statue
Mapobject type			: Stand still 
Date                    	: April 29th 2000
Filename                	: md3-xaero_statue.pk3

Author                  	: Oak
Email Address           	: Oak@abonados.cplus.es
Home Page               	: http://www.planetquake.com/OakShiro/

Program Used			: 3dsMax 3
Additional Ideas / Remarks	: Deathmonger

Main Gameplay Testing		: Deathmonger, Oak


=============================================================================
* About *
=============================================================================
This model is a xaero statue, there are two different versions of it (a Xaero
and a Deathmonger one), the only thing that changes is the skin. You can choose
between a xaero skin based statue, or a deathmonger skin based.
I took the xaero model as base model, created by Kenneth Scott, from Id Software.

=============================================================================
* How to install the model *
=============================================================================
Simply unzip the .pk3 file into your baseq3 directory.

=============================================================================
* How to add this model to your maps *
=============================================================================
Start Q3radiant and open/create the destination map for the model.
Right click wherever you want and select the "misc-model" option.
Close the browser and type this in the "key" and "value" boxes from the 
newborn floating window:

Key: "model"

Value: "models/mapobjects/xaero_statue/xaero_statue.md3" 
       "models/mapobjects/dm_statue/dm_statue.md3"

Depending in which of the two versions you want to add.
=============================================================================
* Permissions *
=============================================================================
This mapobject may be electronically distributed only at no charge to
the recipient, and may not be modified in any way.  This text file
must be included with the level.


----------------------------------------------------------------------------------------------------------------------------------------------------------


date: 22nd March 2001.
Quake3Arena map object models

====================================================
title:            Plant Model Pack II, Palms 
file:             multiplant2.pk3
author:           todd gantzler 
email address:    toddg@slip.net
URL:              http://i.am/professorQ3

description:      REQUIRES multiplant.pk3
		  multiplant2.pk3 is an expansion pack for multiplant.pk3
		  ## palms of various polycounts from 2 to ~500
		  .md3 models and prefabs
====================================================
model information

most of these plants use a basic shader with simple GE128 transparency

i've included prefabs with collision and sound for the original multiplant pack.  
not recomended for the traditionally preferred "smoothly clipped" map.  
all triggers should be set to fire only once every 3-5 sec.

how to use        place multiplant2.pk3 in your /baseq3/ folder
                  choose one of the multiplant models from the model menu
			in Q3Radient
                  
====================================================
construction

base:           none
editor:         3DStudioMAX r3.1
other progs:    Pop'n'Fresh .md3 exporter
                
know bugs:      may consume carbon dioxide, add oxygen to your arena

build time:     these were built over a very long period of time, and 
		i've lost track of time spent actually modeling.

====================================================
thanks to ...
G1zmo for two excellent tall palm prefabs, and model testing
Sovereign for model testing.
Mr. Clean for textures used in these prefabs.

someone for a bark texture or two, the origin of which i've lost track.

Fatmanfat for shader advice and assistance
many kind folks from Quake3world level editing forum
for artistic feedback

the folks at the quake3world level editing and 
editing and modifications forums for shader info
and general technical assistance.

====================================================
Distribution / Copyright / Permissions 

you may use these models, textures, and shaders in your maps.
you may extract and use just the files you need.
please include the following information in your 
readme file:

multiplant3.pk3 - .md3, texture, and shader files by 
Todd Gantzler Copyright (c) 2001
http://i.am/professorQ3
____________________________________________________

all original artwork Copyright (c) 2001
All rights reserved.

based on shader files from Quake III Arena
Copyright (c) 1999 id Software, Inc.
All rights reserved.

Quake III Arena is a registered trademark of 
id Software, Inc.

This file may be electronically distributed only at 
NO CHARGE to the recipient in its current state, MUST 
include this .txt file, and may NOT be modified IN 
ANY WAY. UNDER NO CIRCUMSTANCES ARE THE FILE OR THE 
CONTENTS OF THIS FILE TO BE DISTRIBUTED ON CD-ROM 
WITHOUT PRIOR WRITTEN PERMISSION.

====================================================





date: 22nd July 2000.
Quake3Arena map object model

====================================================
title:            Plant Model Pack I, tropical plants
file:             multiplant.pk3
author:           todd gantzler 
email address:    toddg@slip.net
URL:              http://i.am/professorQ3
description:      a 54 poly multiplant model
			with 8 different skins/shaders
			(in 8 different .md3 files)

====================================================
model information

most of these plants use a basic shader with simple GE128 transparency
(ferns use an invisible shader to eliminate some geometry)

multiplant.md3			a plant whose name i don't remember 
multiplant_b.md3		a banana plant
multiplant_f.md3		a fern 
multiplant_f2.md3		another fern
multiplant_p.md3		a somewhat battered palm
multiplant_p2.md3		another palm
multiplant_a.md3		another plant
multiplant_e.md3		an evil plant meant to hurt the player on contact
				two shader passes plus lightmap, vertex deformation

how to use        place multiplant.pk3 in your /baseq3/ folder
                  choose one of the multiplant models from the model menu
			in Q3Radient
                  
====================================================
construction

base:           none
editor:         3DStudioMAX r3.1
other progs:    Pop'n'Fresh .md3 exporter
                
know bugs:      may consume carbon dioxide, add oxygen to your arena

build time:     a couple of hours of model building, about a week of
			shader/texture tuning

====================================================
thanks to ...

Fatmanfat for shader advice and assistance
Brok3n and Froggyquim for artistic feedback

the folks at the quake3world level editing and 
editing and modifications forums for shader info
and general technical assistance.
thanks to Rungy in particular for technical info.

====================================================
Distribution / Copyright / Permissions 

you may use these models, textures, and shaders in your maps.
you may extract and use just the files you need.
please include the following information in your 
readme file:

multiplant.pk3 - .md3, texture, and shader files by 
Todd Gantzler Copyright (c) 2000
http://i.am/professorQ3
____________________________________________________

all original artwork Copyright (c) 2000
All rights reserved.

based on shader files from Quake III Arena
Copyright (c) 1999 id Software, Inc.
All rights reserved.

Quake III Arena is a registered trademark of 
id Software, Inc.

This file may be electronically distributed only at 
NO CHARGE to the recipient in its current state, MUST 
include this .txt file, and may NOT be modified IN 
ANY WAY. UNDER NO CIRCUMSTANCES ARE THE FILE OR THE 
CONTENTS OF THIS FILE TO BE DISTRIBUTED ON CD-ROM 
WITHOUT PRIOR WRITTEN PERMISSION.

===========================================================================================================================




Todd Gantzler's Plant Model Pack, Version 2.3		03.03.05
Updated by Obsidian (meridanox@gmail.com)
zzz_md3_multiplant_v23.pk3


FOR FULL INFORMATION AND COPYRIGHT INFO, PLEASE REFER TO:
  * legacy_multiplant.txt
  * legacy_multiplant2.txt
  * legacy_readme.txt


INFORMATION
=================================================================
This is an update to Todd Gantzler's multiplant models,
containing all assets from both Plant Model Pack I and II.

I (Obsidian) am redistributing this pack with Todd's permission
with several bug fixes and optimizations.


INSTALLATION
=================================================================
  * Remove any older versions of the Multiplant model pack.
  * Extract zzz_md3_multiplant_v23.pk3 to "baseq3" directory.

  * Note: Do not rename the zzz_md3_multiplant_v23.pk3 file. PK3
    files are loaded into Quake3 in alphabetical order and the
    updated multiplant model pack must be loaded after any pre-
    existing versions to overwrite them.


VERSION HISTORY
=================================================================
2.3 (03.03.05)
  * tallpalm1.map, tallpalm2.map, tallthinpalm.map prefab objects
    rebuilt in 3dsMAX and exported as ASE models (remakes)
  * Resized image files to power of 2 dimensions (larger file
    size but optimized for texture memory and image quality)
  * palm3.md3 - rearranged edge orientation, welded vertexes,
    reapplied smoothing groups, UVW-wrapped, reapplied materials
  * palm4.md3 - welded vertexes, reapplied smoothing groups,
    reapplied materials
  * palm4a.md3 - welded vertexes, reapplied smoothing groups,
    changed leaf materials from palmfrond to bannanaleaf
2.2 (28.02.05)
  * Fixed bug when models are overlapped with alphaMod volume
    brushes (thanks ydnar)
  * Fixed missing shader error (thanks Kat)
  * Rewrote "invisible" shader for dual purpose use with both
    legacy and modern maps
  * Renamed PFB files to MAP for use with GtkRadiant
  * Reattached Todd's prefabs to zip
  * Added multiplant.bsp as a model gallery (/devmap multiplant)

2.1 (03.06.04)
  * Updated to include Plant Model Pack II
  * Palm models processed through MD3Fix.exe
  * palms.shader merged into multiplant.shader
  * "models/mapobjects/multiplant/invisible" shader replaced with
    a modified nodraw shader for reduced polycounts

2.0 (28.05.04)
  * Initial Version 2 release
  * MD3's processed through MD3Fix.exe 0.2 to correct invalid
    shader names generated by Pop'n'Fresh


NOTES FOR DEVELOPERS
=================================================================
Also included for developers are updated versions of Todd's
prefabs, converted to .map format for use with recent versions of
GtkRadiant.

To view the gallery type "/devmap multiplant" in the console. The
sample map was compiled with Q3Map2 2.5.16 with the following
light switches: -light -fast -patchshadows -samples 3 -gamma 2
-compensate 4 -dirty

To add models in your maps, you may need to extract the MD3 files
from the PK3 to your development directory.

Make sure you add "multiplant" to your shaderlist.txt document.

Compile your map using the latest stable version of Q3Map2.

As a result of creating support of legacy maps as well as
optimizing the models for new maps, the "invisible" shader was
improved for this release (2.2+). Legacy maps will display an
invisible shader, but extra polys will be drawn. Newly compiled
maps will make use of the nodraw features of the shader, so the
extra polys will not be drawn in game, resulting in improved
performance.

I noticed that palm4.md3 and palm4a.md3 were the exact same model
so I retextured the leaves of palm4a with the bannanaleaf.tga
texture to give some variation between the models (version 2.3). 


THANKS
=================================================================
Obsidian would like to thank...
  * ydnar for Q3Map2, MD3Fix and help with the updated shaders.
  * Kat for bug testing.
  * Todd Gantzler for the chlorophyll.


MODEL LIST
=================================================================
models/mapobjects/multiplant/
	multiplant.md3
	multiplant_a.md3
	multiplant_b.md3
	multiplant_e.md3
	multiplant_f2.md3
	multiplant_f.md3
	multiplant_p2.md3
	multiplant_p.md3
	tallpalm1.ase
	tallpalm2.ase
	tallthinpalm.ase

models/mapobjects/palm1/
	palm1.md3

models/mapobjects/palm2/
	palm2.md3
	palm2a.md3

models/mapobjects/palm3/
	palm3.md3

models/mapobjects/palm4/
	palm4.md3
	palm4a.md3

maps/
	multiplant.bsp
	multiplant.map

(NOT IN PK3)/prefabs/
	multiplant.map
	multiplant_a.map
	multiplant_b.md3
	multiplant_p2.md3
	multiplant_p.md3
	palm1.map
	palm2.map
	palm3.map
	palm4.map
	palm4a.map
	tallpalm1.map
	tallpalm2.map
	tallthinpalm.map

PK3 Count: 52 files in 12 folders
ZIP Count: 18 files in 2 folders
--------------------------------------------------------------------------------------------------------------------------------------------




date: 22nd March 2001.
Quake3Arena map object models

====================================================
title:            egyptian model pack 
file:             md3-egypt.pk3
author:           todd gantzler 
email address:    toddg@slip.net
URL:              http://i.am/professorQ3

description:      a bunch of models of an egyptian theme
		  most are finished, but some of these models are 
		  FINAL BETA (meaning the model pack is FINAL BETA) 
		  i hope to update them in the near future.
====================================================
model information
		  a bunch of models of an egyptian theme

how to use        place md3-egypt.pk3 in your /baseq3/ folder
                  choose one of the multiplant models from the model menu
			in Q3Radient
                  
====================================================
construction

base:           none
editor:         3DStudioMAX r3.1
other progs:    Pop'n'Fresh .md3 exporter
                
know bugs:      

build time:     these were built over a very long period of time, and 
		i've lost track of time spent actually modeling.

====================================================

the folks at the quake3world level editing and 
editing and modifications forums for shader info
and general technical assistance.

====================================================
Distribution / Copyright / Permissions 

you may use these models, textures, and shaders in your maps.
you may extract and use just the files you need.
please include the following information in your 
readme file:

egypt.pk3 - .md3, texture, and shader files by 
Todd Gantzler Copyright (c) 2001
http://i.am/professorQ3
____________________________________________________

all original artwork Copyright (c) 2001
All rights reserved.

based on shader files from Quake III Arena
Copyright (c) 1999 id Software, Inc.
All rights reserved.

Quake III Arena is a registered trademark of 
id Software, Inc.

This file may be electronically distributed only at 
NO CHARGE to the recipient in its current state, MUST 
include this .txt file, and may NOT be modified IN 
ANY WAY. UNDER NO CIRCUMSTANCES ARE THE FILE OR THE 
CONTENTS OF THIS FILE TO BE DISTRIBUTED ON CD-ROM 
WITHOUT PRIOR WRITTEN PERMISSION.

====================================================

