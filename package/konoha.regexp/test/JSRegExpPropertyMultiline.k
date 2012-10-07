//written by Joseph

import("konoha.class");
import("konoha.regexp");

void test() {
	RegExp patt = /Anything/m;
	assert(patt.multiline == true);
	patt = /Anything/;
	assert(patt.multiline == false);
}

test();
