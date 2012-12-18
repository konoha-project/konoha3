//written by Joseph

import("js4.string");

void test() {
	assert("こんにちは、世界!".search("世界") == 6);
	assert("こんにちは、世界!".search("せかい") == -1);
}

test();
