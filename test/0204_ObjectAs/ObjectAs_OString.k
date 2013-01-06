// written by kimio

import("JavaStyle.Object");

void test() {
	Object o = "123";
	String s = o as String;
	assert(s == "123");
}

test();

