// written by kimio

import("JavaStyle.Object");

void test() {
	String s = "12356";
	assert((s to int) == 12356);
	assert(("12356" to int) == 12356);
	assert("A" to int == 0);  // number string
}

test();

