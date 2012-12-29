//written by Joseph

import("konoha.class");
import("JavaScript.Regexp");

void test() {
	RegExp patt = /Anything/g;
	assert(patt.source == "Anything");
}

test();
