//written by Joseph

import("konoha.class");
import("konoha.regexp");

void test() {
	RegExp patt = /世界/m;
	assert(patt.multiline == true);
	patt = /世界/;
	assert(patt.multiline == false);
}

test();
