// written by kimio

import("konoha.object");

void test() {
	Object o = "123";
	int n = o as int;  // this must cause stupid cast
	assert(n == 123);  // never reach
}

test();

