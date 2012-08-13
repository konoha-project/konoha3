//written by Joseph

K.import("konoha.regexp");
K.import("konoha.assign");

void test() {
	RegExp patt = /Anything/m;
	assert(patt.multiline == true);
	patt = /Anything/g;
	patt.compile(patt);
	assert(patt.multiline == false);
}

test();
