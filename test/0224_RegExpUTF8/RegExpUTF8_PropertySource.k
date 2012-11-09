//written by Joseph

import("konoha.class");
import("konoha.regexp");

void test() {
	RegExp patt = /こんにちは/g;
	assert(patt.source == "こんにちは");
}

test();
