//written by Joseph

import("JavaScript.String");

void test() {
	assert("こんにちは、世界!".substr(2) == "にちは、世界!");
	assert("こんにちは、世界!".substr(2, 3) == "にちは");
	assert("こんにちは、世界!".substr(2, 0) == "");
	//"こんにちは、世界!".substr(20, 3); // throws Exception
	assert("こんにちは、世界!".substr(-6) == "ちは、世界!");
	assert("こんにちは、世界!".substr(-6, 9) == "ちは、世界!");
	assert("こんにちは、世界!".substr(-6, 3) == "ちは、");
	//"こんにちは、世界!".substr(-20, 3); // throws Exception
}

test();
