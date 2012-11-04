// written by kimio

import("konoha.object");

void test() {
	Object o = 123;
	assert(o to int == 123);
	assert(o to String == "123");
}

test();

