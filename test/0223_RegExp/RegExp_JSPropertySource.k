//written by Joseph

import("konoha.class");
import("js4.regexp");

void test() {
	RegExp patt = /Anything/g;
	assert(patt.source == "Anything");
}

test();
