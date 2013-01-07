// written by kimio

import("JavaStyle.Object");

void test() {
	String s = "12356";
	assert((s to String) == "12356");
	assert(("12356" to String) == "12356");
}

test();

