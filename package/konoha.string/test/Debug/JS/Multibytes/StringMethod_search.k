//written by Joseph

K.import("konoha.string");
K.import("konoha.regexp");

void test() {
	assert("こんにちは、世界!".search("世界") == 6);
	assert("こんにちは、世界!".search("せかい") == -1);
	assert("こんにちは、世界!".search(/世界/gi) == 6);
	assert("こんにちは、世界!".search(/世界/g) == 6);
	assert("こんにちは、世界!".search(/世界/i) == 6);
}

test();
