//written by Joseph

K.import("konoha.string");
K.import("konoha.regexp");

void test() {
	assert("Hello, world!".search("world") == 7);
	assert("Hello, world!".search("World") == -1);
	assert("Hello, world!".search(/World/gi) == 7);
	assert("Hello, world!".search(/World/g) == -1);
	assert("Hello, world!".search(/World/i) == 7);
}

test();
