// written by Joseph
// modified by chenji

import("JavaScript.String");

void test() {
	assert("Hello, world!".slice(1, 5) == "ello");
	assert("Hello, world!".slice(1, -2) == "ello, worl");
	assert("Hello, world!".slice(3, 2) == "");
	assert("Hello, world!".slice(5) == ", world!");
	assert("Hello, world!".slice(-5) == "orld!");
	assert("Hello, world!".slice(-3, -6) == "");
	assert("Hello, world!".slice(-6, -3) == "wor");
}

test();
