//written by Joseph

import("konoha.string");

void test() {
	assert("Hello, world!".search("world") == 7);
	assert("Hello, world!".search("World") == -1);
}

test();
