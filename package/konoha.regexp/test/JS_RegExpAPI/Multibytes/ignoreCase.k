//written by Joseph

K.import("konoha.regexp");
K.import("konoha.assign");

void test() {
	//check whether or not "i" modifier is set.
	RegExp patt = /世界/i;
	assert(patt.global == true);
	patt = /世界/g;
	patt.compile(patt);
	assert(patt.global == false);
}

test();
