var STRUCT_LAYOUT_INFO = {
    bspShader_t: {
        name: 0, // char[64]
        surfaceFlags: 64, // int
        contentsFlags: 68, // int
        SIZEOF: 72
    },

    bspFace_t: {
        shader: 0, // int
        fog: 4, // int
        type: 8, // int
        vertexOffset: 12, // int
        numVertexes: 16, // int
        meshVertexOffset: 20, // int
        numMeshVertexes: 24, // int
        lightmapIndex: 28, // int
        lightmapStartCorner: 32, // int[2]
        lightmapSize: 40, // int[2]
        lightmapOrigin: 48, // float[3]
        lightmapStVecs: 60, // float[2][3],
        normal: 84, // float[3]
        patchDimensions: 96, // int[2]
        SIZEOF: 104
    },

    mapData_t: {
        header: 0,
        entitiesSrc: 144,
        shaders: 156,
        planes: 168,
        nodes: 180,
        leaves: 192,
        leafFaces: 204,
        models: 228,
        brushes: 240,
        brushSides: 252,
        vertexes: 264,
        meshVertexes: 276,
        fogs: 288,
        faces: 300,
        lightmaps: 312,
        lightvols: 324,
        bitsetSrc: 336,
        entities: 348,
        visdata: 352,
        entityStringLen: 360,
        numNodes: 364,
        numLeaves: 368,
        numLeafFaces: 372,
        numLeafBrushes: 376,
        numPlanes: 380,
        numVertexes: 384,
        numBrushes: 388,
        numBrushSides: 392,
        numShaders: 396,
        numModels: 400,
        numFogs: 404,
        numFaces: 408,
        numMeshVertexes: 412,
        numLightmaps: 416,
        numLightvols: 420,
        numVisdataVecs: 424,
        SIZEOF: 428
    },

    shaderInfo_t: {
        deform: 0,  // bool
        deformCmd: 4,   // vertexDeformCmd_t
        deformFn: 8,    // vertexDeformFunc_t
        deformParms: 12,    // effect_t
        cullFace: 52,   // uint32_t
        surfaceParms: 56,   // uint32_t
        localLoadFlags: 60,   // uint8_t
        tessSize: 64,   // float
        stageCount: 68,     // int
        surfaceLight: 72,   // float
        name: 76,       // std::array<char, BSP_MAX_SHADER_TOKEN_LENGTH>
        stageBuffer: 140,   // std::vector<shaderStage_t>
        SIZEOF: 152
    }
};

var CLASS_LAYOUT_INFO = {
    BSPRenderer: {
        map: 32         // Q3BspMap
    },

    Q3BspMap: {
        name: 8,        // std::string
        data: 68        // mapData_t
    },

    // see line 2138
    // allocator_traits has no actual members,
    // just definitions/static methods.
    // same with __vector_base_common
    vector: {
        storage: 0,     // __storage_pointer
        size: 4         // size_type
    }
};

var BSP_CONTENTS_FLAGS = {
    SOLID: 0x1,
	LAVA: 0x8,
	SLIME: 0x10,
	WATER: 0x20,
	FOG: 0x40,

	NOTTEAM1: 0x0080,
	NOTTEAM2: 0x0100,
	NOBOTCLIP: 0x0200,

	AREAPORTAL: 0x8000,
	PLAYERCLIP: 0x10000,
	MONSTERCLIP: 0x20000,

	TELEPORTER: 0x40000,
	JUMPPAD: 0x80000,
	CLUSTERPORTAL: 0x100000,
	DONOTENTER: 0x200000,
	BOTCLIP: 0x400000,
	MOVER: 0x800000,

	ORIGIN: 0x1000000, // removed before bsping an entity

	BODY: 0x2000000,
	CORPSE: 0x4000000,
	DETAIL: 0x8000000, // brush: not used for the bsp
	STRUCTURAL: 0x10000000, // brush: used for the bsp
	TRANSLUCENT: 0x20000000, // don't consume surface fragments inside
	TRIGGER: 0x40000000, //
	NODROP: 0x80000000
};

var BSP_SURFACE_FLAGS = {
    NODAMAGE: 0x1, // never give falling damage
    SLICK: 0x2, // effects game physics
    SKY: 0x4, // lighting from environment map
    LADDER: 0x8,
    NOIMPACT: 0x10, // don't make missile explosions
    NOMARKS: 0x20, // don't leave missile marks
    FLESH: 0x40, // make flesh sounds and effects
    NODRAW: 0x80, // don't generate a drawsurface at all
    HINT: 0x100, // make a primary bsp splitter
    SKIP: 0x200, // completely ignore, allowing non-closed brushes
    NOLIGHTMAP: 0x400, // surface doesn't need a lightmap
    POINTLIGHT: 0x800, // generate light info at vertexes
    METALSTEPS: 0x1000, // clanking footsteps
    NOSTEPS: 0x2000, // no footstep sounds
    NONSOLID: 0x4000, // don't collide against curves with this set
    LIGHTFILTER: 0x8000, // act as a light filter during q3map
    ALPHASHADOW: 0x10000, // do per-pixel light shadow casting in q3map
    NODLIGHT: 0x20000, // don't dlight(?) even if solid (solid lava, skies)
    DUST: 0x40000 // leave a dust trail when walking
};

var getHeapString = function(address) {
    var counter = address;
    var str = '';
    while (HEAP8[counter] != 0) {
        str += String.fromCharCode(HEAP8[counter++]);
    }
    return str;
};

var dumpJSON = function(obj) {
    console.log(JSON.stringify(obj, null, 4));
};

var bspFace_t_elem = function(base, key, ofs, heap32) {
    ofs = ofs || 0;
    heap32 = heap32 || HEAP32;
    return heap32[(base + STRUCT_LAYOUT_INFO.bspFace_t[key] + ofs) >> 2];
};

var printHeapString = function(address) {
    console.log(getHeapString(address));
};

var BSPRenderer_map = function($pRenderer) {
    return HEAP32[($pRenderer + 32) >> 2];
};

var BSPRenderer_map_data = function($pRenderer) {
    return Q3BspMap_data(BSPRenderer_map($pRenderer));
}

var Q3BspMap_name = function($pMap) {
    return $pMap + 8;
};

var Q3BspMap_data = function($pMap) {
    return $pMap + 68;
};

var mapData_t_shaders = function($pData) {
    return HEAP32[($pData + STRUCT_LAYOUT_INFO.mapData_t.shaders) >> 2];
};

var mapData_t_faces = function($pData) {
    return HEAP32[($pData + STRUCT_LAYOUT_INFO.mapData_t.faces) >> 2];
};

var makeFlagNameList = function(object, flag) {
    var list = [];

    for (var key in object) {
        if ((flag & object[key]) !== 0) {
            list.push(key);
        }
    }

    return list;
};

var addListNames = function(listTitle, list) {
    var outBuffer = listTitle + '\n';

    if (list.length == 0) {
        outBuffer += '\t<NONE>\n';
    } else {
        for (var i = 0; i < list.length; ++i) {
            outBuffer += '\t' + list[i] + '\n';
        }
    }

    return outBuffer;
};

var mapData_t_getShaderInfo = function($pData, index) {
    var offset = index * STRUCT_LAYOUT_INFO.bspShader_t.SIZEOF;

    var shadersBuff = mapData_t_shaders($pData);

    var pElem = shadersBuff + offset;

    var name = getHeapString(pElem + STRUCT_LAYOUT_INFO.bspShader_t.name);
    var surfaceFlags = HEAP32[(pElem + STRUCT_LAYOUT_INFO.bspShader_t.surfaceFlags) >> 2];
    var contentFlags = HEAP32[(pElem + STRUCT_LAYOUT_INFO.bspShader_t.contentsFlags) >> 2];

    var surfaceFlagNames = makeFlagNameList(BSP_SURFACE_FLAGS, surfaceFlags);
    var contentsFlagNames = makeFlagNameList(BSP_CONTENTS_FLAGS, contentFlags);

    var outString = '';

    outString += '[NAME]\n\t' + name + '\n';

    outString += addListNames('[SURFACE FLAGS]', surfaceFlagNames);
    outString += addListNames('[CONTENTS FLAGS]', contentsFlagNames);

    return outString;
};

var mapData_t_getFaceInfo = function($pData, index) {
    var offset = index * STRUCT_LAYOUT_INFO.bspFace_t.SIZEOF;

    var facesBuff = mapData_t_faces($pData);

    var pElem = facesBuff + offset;

    var info = {
        shader: bspFace_t_elem(pElem, 'shader'), // int
        fog: bspFace_t_elem(pElem, 'fog'), // int
        type: bspFace_t_elem(pElem, 'type'), // int
        vertexOffset: bspFace_t_elem(pElem, 'vertexOffset'), // int
        numVertexes: bspFace_t_elem(pElem, 'numVertexes'), // int
        meshVertexOffset: bspFace_t_elem(pElem, 'meshVertexOffset'), // int
        numMeshVertexes: bspFace_t_elem(pElem, 'numMeshVertexes'), // int
        lightmapIndex: bspFace_t_elem(pElem, 'lightmapIndex'), // int
        lightmapStartCorner: [
            bspFace_t_elem(pElem, 'lightmapStartCorner'),  // int[2]
            bspFace_t_elem(pElem, 'lightmapStartCorner', 4)
        ],
        lightmapSize: [
            bspFace_t_elem(pElem, 'lightmapSize'), // int[2]
            bspFace_t_elem(pElem, 'lightmapSize', 4)
        ],
        lightmapOrigin: [ 
            bspFace_t_elem(pElem, 'lightmapOrigin', 0, HEAPF32), // float[3]
            bspFace_t_elem(pElem, 'lightmapOrigin', 4, HEAPF32),
            bspFace_t_elem(pElem, 'lightmapOrigin', 8, HEAPF32)
        ],
        lightmapStVecs: [
            bspFace_t_elem(pElem, 'lightmapStVecs', 0, HEAPF32), // float[2][2],
            bspFace_t_elem(pElem, 'lightmapStVecs', 4, HEAPF32),
            bspFace_t_elem(pElem, 'lightmapStVecs', 8, HEAPF32),
            bspFace_t_elem(pElem, 'lightmapStVecs', 12, HEAPF32),
            bspFace_t_elem(pElem, 'lightmapStVecs', 16, HEAPF32),
            bspFace_t_elem(pElem, 'lightmapStVecs', 20, HEAPF32)
        ],
        normal: [
            bspFace_t_elem(pElem, 'normal', 0, HEAPF32), // float[3]
            bspFace_t_elem(pElem, 'normal', 4, HEAPF32),
            bspFace_t_elem(pElem, 'normal', 8, HEAPF32)
        ],
        patchDimensions: [
            bspFace_t_elem(pElem, 'patchDimensions'),
            bspFace_t_elem(pElem, 'patchDimensions', 4) // int[2]
        ]
    };

    dumpJSON(info);

    return info;
};

var shaderInfo_t_printName = function($pShader) {
    printHeapString($pShader + STRUCT_LAYOUT_INFO.shaderInfo_t.name);
};

var stringLen = function(address) {
	var c = address;
	while (HEAP8[c] != 0) {
		c++;
	}
	return c - address;
};

var getStdString_c_str = function($thisPtr) {
	var isShort = HEAP8[$thisPtr];
	var test = (isShort & 0x1) == 0;
	if (test) {
		return $thisPtr + 1;
	} else {
		return HEAP32[($thisPtr + 8) >> 2];
	}
};

var printStdString = function($thisPtr) {
	printHeapString(getStdString_c_str($thisPtr));
};

var get8 = function(address, n) {
	var ret = [];

	for (var x = 0; x < n; ++x) {
		ret.push(HEAP8[address + x]);
	}

	return ret;
};

var get32 = function(address, n) {
	var ret = [];

	address = address >> 2;

	for (var x = 0; x < n; ++x) {
		ret.push(HEAP32[address + x]);
	}

	return ret;
};

var dumpStdString = function($this, extraLen, before) {
	before = before || 0;
	var cStringAddress = getStdString_c_str($this);
	var n = stringLen(cStringAddress) + (extraLen || 0);
	return get8( cStringAddress - before, n );
};

var getHexString = function(str) {
	var hex = '';
	for (var x = 0; x < str.length; ++x) {
		hex += str.charCodeAt(x) + ' ';
	}
	return hex;
};

var buildStringFromU8 = function(array, start, end) {
	if (!end) {
		end = Number.MAX_SAFE_INTEGER >> 50; // so we don't potentially wait for an eternity
	}
	let str = "";
	for (let i = start; array[i] !== 0 && i < array.length && i < end; ++i) {
		str += String.fromCharCode(array[i]);
	}
	return str;
};

var dumpMem8 = function(ptr, len, lineLength) {
    let bytes = '';
    let ascii = '';
    let all = '';
    let lineCount = 0;

    lineLength = lineLength || 6;

    for (let i = 0; i < len; ++i) {
        let b = HEAP8[ptr + i];

        let n0 = b & 0xF;
        let n1 = (b & 0xF0) >> 4;

    //    console.log('n0: ', n0, ', n1: ', n1);

        bytes += '0x' + n1.toString(16) + n0.toString(16) + ' ';
        if (0x20 <= b && b <= 0x7E) {
            ascii += String.fromCharCode(b);
        } else {
            ascii += '.';
        }

        if ((i + 1) % lineLength == 0) {
            all += '[\t' + lineCount + '\t]' + bytes + '\t|\t' + ascii + '\n';
            bytes = '';
            ascii = '';
            lineCount++;
        }
    }

    return '\n\n' + all + '\n\n';
}
