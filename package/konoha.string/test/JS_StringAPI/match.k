//written by Joseph

K.import("konoha.string");
K.import("konoha.regexp");

void test() {
	assert("The rain in SPAIN stays mainly in the plain".match(/ain/g) == "ain,ain,ain");
	assert("The rain in SPAIN stays mainly in the plain".match(/ain/gi) == "ain,AIN,ain,ain");
	RegExp patt = /[A-C]/gi;
	assert("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefgijklmnopqrstuvwxyz".match(patt) == "'A','B','C','a','b','c'");
	assert("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefgijklmnopqrstuvwxyz".match(/[A-C]/g) == "'A','B','C'");
}

test();
