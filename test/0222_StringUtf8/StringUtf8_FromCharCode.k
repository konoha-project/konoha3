//written by Joseph

import("JavaScript.String");

void test() {
	assert(String.fromCharCode(12371) == "こ");
	assert(String.fromCharCode(12435) == "ん");
	assert(String.fromCharCode(12395) == "に");
	assert(String.fromCharCode(12385) == "ち");
	assert(String.fromCharCode(12399) == "は");
	//assert(String.fromCharCode(12371, 12435, 12395, 12385, 12399) == "こんにちは");
}

test();
