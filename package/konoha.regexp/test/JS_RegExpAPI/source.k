//written by Joseph

K.import("konoha.regexp");
K.import("konoha.assign");

void test() {
	RegExp patt = /Anything/g;
	assert(patt.source == "Anything");
}

test();
