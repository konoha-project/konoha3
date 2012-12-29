//written by Joseph

import("JavaScript.Regexp");

void test() {
	assert("Hello, world!".search(/World/gi) == 7);
	assert("Hello, world!".search(/World/g) == -1);
	assert("Hello, world!".search(/World/i) == 7);
}

test();
