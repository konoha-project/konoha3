//written by Joseph

import("konoha.regexp");
import("konoha.assign");
import("konoha.class");

void test() {
	RegExp patt = /こんにちは/g;
	assert(patt.source == "こんにちは");
}

test();
