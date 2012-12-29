// written by Joseph
// modified by chenji

import("JavaScript.String");

void test() {
	assert("Hello world!".substr(2) == "llo world!");
	assert("Hello world!".substr(2, 3) == "llo");
	assert("Hello world!".substr(2, 0) == "");
	assert("Hello world!".substr(-6) == "world!");
	assert("Hello world!".substr(-6, 6) == "world!");
	assert("Hello world!".substr(-6, 3) == "wor");
}

test();
