//written by Joseph

K.import("konoha.regexp");
K.import("konoha.assign");

void test() {
	RegExp patt = /こんにちは/g;
	assert(patt.source == "こんにちは");
}

test();
