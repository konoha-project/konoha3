// written by kimio

import("konoha.object");

void test() {
	int n = 123;
	Object o = n;
	assert(o.toString() == "123");
}

test();
