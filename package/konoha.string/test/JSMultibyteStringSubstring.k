//written by Joseph

import("konoha.string");

void test() {
	assert("こんにちは、世界!".substring(3, 7) == "ちは、世");
	assert("こんにちは、世界!".substring(7, 3) == "ちは、世");
	assert("こんにちは、世界!".substring(0, 10) == "こんにちは、世界!");
	//NOTE: The length of "こんにちは、世界!" is 9.
	assert("こんにちは、世界!".substring(3, 15) == "ちは、世界!");

	assert("こんにちは、世界!".substring(3) == "ちは、世界!");
	//assert("こんにちは、世界!".substring(10) == "");
	assert("こんにちは、世界!".substring(0) == "こんにちは、世界!");
	assert("こんにちは、世界!".substring(-10) == "こんにちは、世界!");

	assert("こんにちは、世界!".substring(-5, 5) == "こんにちは");
	assert("こんにちは、世界!".substring(-5, -3) == "");
	assert("こんにちは、世界!".substring(4, -3) == "こんにち");
	assert("こんにちは、世界!".substring(5, 10) == "、世界!");
}

test();
