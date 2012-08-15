//written by Joseph

K.import("konoha.string");

void test() {
	assert("Hello world.".indexOf("world") == 6);
	assert("Hello world.".indexOf("worlde") == -1);
	assert("Hello world.".indexOf("world", 0) == 6);
	assert("Hello world.".indexOf("world", 5) == 6);
	assert("Hello world.".indexOf("world", 9) == -1);
	//NOTE: The length of "Hello world." is 11
	assert("Hello world.".indexOf("", 10) == 10);
	assert("Hello world.".indexOf("", 11) == 11);
	assert("Hello world.".indexOf("", 12) == 11);
	assert("Hello world.".indexOf("WORLD") == -1);
	//Check if a specific string exists within the string.
	assert(("Hello world.".indexOf("world") != -1) == true);
	assert(("Hello world.".indexOf("WORLD") != -1) == false);
}

test();
