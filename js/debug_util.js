var printHeapString = function(address) {
    var counter = address;
    var str = '';
    while (HEAP8[counter] != 0) {
        str += String.fromCharCode(HEAP8[counter++]);
    }
    console.log(str);
	return str;
};

var getHexString = function(str) {
	var hex = '';
	for (var x = 0; x < str.length; ++x) {
		hex += str.charCodeAt(x) + ' ';
	}
	return hex;
}
