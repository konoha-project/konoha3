//written by Joseph

K.import("konoha.regexp");
K.import("konoha.assign");
K.import("konoha.class");

void test() {
	//check whether or not "g" modifier is set.
	RegExp patt = /世界/g;
	assert(patt.global == true);
	patt = /世界/;
	assert(patt.global == false);
}

test();
