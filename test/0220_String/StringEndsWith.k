//written by takeo onoda

import("java2.string");

void test() {
	assert("foobar".endsWith("bar") == true);
	assert("foobar".endsWith("foobar") == true);
	assert("foobar".endsWith("0foobar") == false);
