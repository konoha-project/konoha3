//written by Joseph

import("konoha.string");

void test() {
	assert("こんにちは、世界!".substr(2) == "にちは、世界!");
	assert("こんにちは、世界!".substr(2, 3) == "にちは");
	assert("こんにちは、世界!".substr(2, 0) == "");
	assert("こんにちは、世界!".substr(3, 20) == "ちは、世界!");
	//"こんにちは、世界!".substr(20, 3); // throws Exception
	assert("こんにちは、世界!".substr(-6) == "ちは、世界!");
	assert("こんにちは、世界!".substr(-6, 9) == "ちは、世界!");
	assert("こんにちは、世界!".substr(-6, 3) == "ちは、");
	//"こんにちは、世界!".substr(-20, 3); // throws Exception
}

test();
