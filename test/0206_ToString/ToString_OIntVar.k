// written by kimio

import("JavaStyle.Object");

void test() {
	int n = 123;
	Object o = n;
	assert(o.toString() == "123");
}

test();
