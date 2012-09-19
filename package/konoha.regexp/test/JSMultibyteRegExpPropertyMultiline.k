//written by Joseph

import("konoha.regexp");
import("konoha.assign");
import("konoha.class");

void test() {
	RegExp patt = /世界/m;
	assert(patt.multiline == true);
	patt = /世界/;
	assert(patt.multiline == false);
}

test();
