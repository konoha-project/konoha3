// written by Joseph
// modified by chenji

import("Java.String");

void test() {
	//NOTE: The length of "Hello world." is 12
	assert("Hello world.".indexOf("world") == 6);
	assert("Hello world.".indexOf("worlde") == -1);
	assert("Hello world.".indexOf("WORLD") == -1);
	assert("Hello world.".indexOf("world", 0) == 6);
	assert("Hello world.".indexOf("world", 5) == 6);
	assert("Hello world.".indexOf("world", 9) == -1);
	assert("Hello world.".indexOf("", 10) == 10);
	assert("Hello world.".indexOf("", 12) == 12);
}

test();
