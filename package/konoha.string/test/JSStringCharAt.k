//written by Joseph

import("konoha.string");

void test() {
	assert("Hello, world".charAt(0) == "H");
	assert("Hello, world".charAt(1) == "e");
	assert("Hello, world".charAt(2) == "l");
	assert("Hello, world".charAt(3) == "l");
	assert("Hello, world".charAt(4) == "o");
	assert("Hello, world".charAt(999) == "");
}

test();
