var printHeapString = function(address) {
    var counter = address;
    var str = '';
    while (HEAP8[counter] != 0) {
        str += String.fromCharCode(HEAP8[counter++]);
    }
    console.log(str);
	return str;
};

var stringLen= function(address) {
	var c = address;
	while (HEAP8[c] != 0) {
		c++;
	}
	return c - address;
}

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
		end = Number.MAX_SAFE_INTEGER >> 32; // so we don't wait for an eternity
	}
	let str = "";
	for (let i = start; array[i] !== 0 && i < array.length && i < end; ++i) {
		str += String.fromCharCode(array[i]);
	}
	return str;
}