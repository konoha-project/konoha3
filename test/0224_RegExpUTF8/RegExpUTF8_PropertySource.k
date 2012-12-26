//written by Joseph

import("konoha.class");
import("js4.regexp");

void test() {
	RegExp patt = /こんにちは/g;
	assert(patt.source == "こんにちは");
}

test();
