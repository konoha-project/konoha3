//written by Joseph

import("konoha.class");
import("konoha.regexp");

void test() {
	RegExp patt = /Anything/g;
	assert(patt.source == "Anything");
}

test();
