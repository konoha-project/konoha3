// written by kimio

import("konoha.object");

void test() {
	Object o = "123";
	String s = o as String;
	assert(s == "123");
}

test();

