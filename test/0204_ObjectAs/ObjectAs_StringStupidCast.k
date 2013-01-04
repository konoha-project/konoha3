// written by kimio

import("JavaStyle.Object");

void test() {
	String s = "123";
	int n = s as int;  // stupid cast
	assert(n == 123);  // never reached
}

test();

