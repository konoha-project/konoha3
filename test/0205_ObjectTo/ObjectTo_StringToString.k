// written by kimio

import("konoha.object");

void test() {
	String s = "12356";
	assert((s to String) == "12356");
	assert(("12356" to String) == "12356");
}

test();

