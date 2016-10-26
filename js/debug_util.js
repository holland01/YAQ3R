var printHeapString = function(address) {
    var counter = address;
    var str = '';
    while (HEAP8[counter] != 0) {
        str += String.fromCharCode(HEAP8[counter++]);
    }
    console.log(str);
	return str;
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

var getHexString = function(str) {
	var hex = '';
	for (var x = 0; x < str.length; ++x) {
		hex += str.charCodeAt(x) + ' ';
	}
	return hex;
};
