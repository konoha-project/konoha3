//written by Joseph

import("konoha.regexp");
import("konoha.assign");
import("konoha.class");

void test() {
	RegExp patt = /Anything/m;
	assert(patt.multiline == true);
	patt = /Anything/;
	assert(patt.multiline == false);
}

test();
