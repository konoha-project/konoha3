//written by Joseph

import("konoha.regexp");
import("konoha.assign");
import("konoha.class");

void test() {
	RegExp patt = /Anything/g;
	assert(patt.source == "Anything");
}

test();
