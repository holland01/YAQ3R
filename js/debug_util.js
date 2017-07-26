var BSP_STRUCT_SIZES = {
    bspShader_t: {
        name: 0,
        surfaceFlags: 64,
        contentsFlags: 68,
        SIZEOF: 72
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

var printHeapString = function(address) {
    console.log(getHeapString(address));
};

var BSPRenderer_map = function($pRenderer) {
    return HEAP32[($pRenderer + 32) >> 2];
};

var Q3BspMap_name = function($pMap) {
    return $pMap + 8;
};

var Q3BspMap_data = function($pMap) {
    return $pMap + 68;
};

var mapData_t_shaders = function($pData) {
    return HEAP32[($pData + 156) >> 2];
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
    var offset = index * BSP_STRUCT_SIZES.bspShader_t.SIZEOF;

    var shadersBuff = mapData_t_shaders($pData);

    var pElem = shadersBuff + offset;

    var name = getHeapString(pElem + BSP_STRUCT_SIZES.bspShader_t.name);
    var surfaceFlags = HEAP32[(pElem + BSP_STRUCT_SIZES.bspShader_t.surfaceFlags) >> 2];
    var contentFlags = HEAP32[(pElem + BSP_STRUCT_SIZES.bspShader_t.contentsFlags) >> 2];

    var surfaceFlagNames = makeFlagNameList(BSP_SURFACE_FLAGS, surfaceFlags);
    var contentsFlagNames = makeFlagNameList(BSP_CONTENTS_FLAGS, contentFlags);

    var outString = '';

    outString += '[NAME]\n\t' + name + '\n';

    outString += addListNames('[SURFACE FLAGS]', surfaceFlagNames);
    outString += addListNames('[CONTENTS FLAGS]', contentsFlagNames);

    return outString;
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
		end = Number.MAX_SAFE_INTEGER >> 32; // so we don't potentially wait for an eternity
	}
	let str = "";
	for (let i = start; array[i] !== 0 && i < array.length && i < end; ++i) {
		str += String.fromCharCode(array[i]);
	}
	return str;
};
