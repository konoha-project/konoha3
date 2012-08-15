//written by Joseph

K.import("konoha.regexp");
K.import("konoha.assign");

void test() {
	//check whether or not "g" modifier is set.
	RegExp patt = /W3S/g;
	assert(patt.global == true);
	patt = /W3S/;
	patt.compile(patt);
	assert(patt.global == false);
}

test();
