//written by Joseph

K.import("konoha.regexp");
K.import("konoha.assign");

void test() {
	//check whether or not "g" modifier is set.
	RegExp patt = /世界/g;
	assert(patt.global == true);
	patt = /世界/;
	patt.compile(patt);
	assert(patt.global == false);
}

test();
