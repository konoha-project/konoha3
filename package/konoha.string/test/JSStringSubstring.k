//written by Joseph

import("konoha.string");

void test() {
	assert("Hello world!".substring(3, 7) == "lo w");
	assert("Hello world!".substring(7, 3) == "lo w");
	assert("Hello world!".substring(0, 10) == "Hello worl");
	//NOTE: The length of "Hello world!" is 12.
	assert("Hello world!".substring(3, 15) == "lo world!");
}

test();
