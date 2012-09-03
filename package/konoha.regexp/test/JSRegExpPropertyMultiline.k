//written by Joseph

K.import("konoha.regexp");
K.import("konoha.assign");
K.import("konoha.class");

void test() {
	RegExp patt = /Anything/m;
	assert(patt.multiline == true);
	patt = /Anything/;
	assert(patt.multiline == false);
}

test();
