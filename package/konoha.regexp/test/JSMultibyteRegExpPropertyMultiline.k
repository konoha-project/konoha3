//written by Joseph

K.import("konoha.regexp");
K.import("konoha.assign");
K.import("konoha.class");

void test() {
	RegExp patt = /世界/m;
	assert(patt.multiline == true);
	patt = /世界/;
	assert(patt.multiline == false);
}

test();
