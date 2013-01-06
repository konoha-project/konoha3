//written by Joseph

import("Java.String");

void test() {
	assert("こんにちは、世界".charAt(0) == "こ");
	assert("こんにちは、世界".charAt(1) == "ん");
	assert("こんにちは、世界".charAt(2) == "に");
	assert("こんにちは、世界".charAt(3) == "ち");
	assert("こんにちは、世界".charAt(4) == "は");
	//"こんにちは、世界".charAt(999); // throws Exception
}

test();
