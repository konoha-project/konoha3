//written by Joseph

import("konoha.class");
import("JavaScript.Regexp");

void test() {
	RegExp patt = /こんにちは/g;
	assert(patt.source == "こんにちは");
}

test();
