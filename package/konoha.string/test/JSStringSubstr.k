//written by Joseph

import("konoha.string");

void test() {
	assert("Hello world!".substr(2) == "llo world!");
	assert("Hello world!".substr(2, 3) == "llo");
	assert("Hello world!".substr(2, 0) == "");
	assert("Hello world!".substr(2, 20) == "llo world!");
	assert("Hello world!".substr(20, 3) == "");
	assert("Hello world!".substr(-6) == "world!");
	assert("Hello world!".substr(-6, 9) == "world!");
	assert("Hello world!".substr(-6, 3) == "wor");
	assert("Hello world!".substr(-20, 3) == "llo");
}

test();
