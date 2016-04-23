var address = 0;
var counter = 0;
var str = '';

while (HEAP8[counter] != 0) {
	str += String.fromCharCode(HEAP8[counter++]);
}

console.log(str);
