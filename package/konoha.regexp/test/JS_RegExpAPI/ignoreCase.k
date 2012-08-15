//written by Joseph

K.import("konoha.regexp");
K.import("konoha.assign");

void test() {
	//check whether or not "i" modifier is set.
	RegExp patt = /W3S/i;
	assert(patt.global == true);
	patt = /W3S/g;
	patt.compile(patt);
	assert(patt.global == false);
}

test();
