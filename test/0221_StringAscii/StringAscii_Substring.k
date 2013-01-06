// written by Joseph
// written by chenji

import("Java.String");

void test() {
	//NOTE: The length of "Hello world!" is 12.
	assert("Hello world!".substring(3, 7) == "lo w");
	//assert("Hello world!".substring(7, 3) == "lo w");  // boundary test shoule be done in another category
	assert("Hello world!".substring(0, 10) == "Hello worl");
}

test();
