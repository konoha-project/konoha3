// written by Joseph
// modified by chenji

import("JavaScript.String");

void test() {
	//NOTE: The length of "こんにちは、世界!" is 9.
	assert("こんにちは、世界!".substring(3, 7) == "ちは、世");
	//assert("こんにちは、世界!".substring(7, 3) == "ちは、世");
	assert("こんにちは、世界!".substring(3) == "ちは、世界!");
	assert("こんにちは、世界!".substring(0) == "こんにちは、世界!");
}

test();
