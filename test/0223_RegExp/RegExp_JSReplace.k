//written by Joseph

import("JavaScript.Regexp");

void test() {
	RegExp patt = /world/gi;
	assert("Hello World, hello world.".replace(patt, "Japan") == "Hello Japan, hello Japan.");
	assert("Hello World, hello world.".replace(/world/gi, "Japan") == "Hello Japan, hello Japan.");
	assert("Hello World, hello world.".replace(/world/g, "Japan") == "Hello World, hello Japan.");
	assert("Hello World, hello world.".replace(/world/i, "Japan") == "Hello Japan, hello world.");
	assert("Hello World!".replace(/(\w+)\s(\w+)/, "$2, $1") == "World, Hello!");
}

test();
