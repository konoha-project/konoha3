//written by Joseph

import("JavaScript.String");

void test() {
	assert("こんにちは、世界!".slice(1, 5) == "んにちは");
	assert("こんにちは、世界!".slice(1, -2) == "んにちは、世");

	assert("こんにちは、世界!".slice(5) == "、世界!");
	assert("こんにちは、世界!".slice(-5) == "は、世界!");

	assert("こんにちは、世界!".slice(-3, -6) == "");
	assert("こんにちは、世界!".slice(-6, -3) == "ちは、");
}

test();
