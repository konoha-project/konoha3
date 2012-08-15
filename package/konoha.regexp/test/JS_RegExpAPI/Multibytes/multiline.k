//written by Joseph

K.import("konoha.regexp");
K.import("konoha.assign");

void test() {
	RegExp patt = /世界/m;
	assert(patt.multiline == true);
	patt = /世界/g;
	patt.compile(patt);
	assert(patt.multiline == false);
}

test();
