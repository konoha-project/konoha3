// written by kimio

import("konoha.object");

void test() {
	int o = 123;
	String s = o as String;  // this must cause donw cast
	assert(s == "");
}

test();

