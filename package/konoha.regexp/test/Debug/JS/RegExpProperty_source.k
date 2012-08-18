//written by Joseph

K.import("konoha.regexp");
K.import("konoha.assign");
K.import("konoha.class");

void test() {
	RegExp patt = /Anything/g;
	assert(patt.source == "Anything");
}

test();
