//written by Joseph

import("JavaScript.String");

void test() {
	assert("こんにちは、にほん".lastIndexOf("に") == 6);
	assert("こんにちは、にほん".lastIndexOf("に", 2) == 2);
	assert("こんにちは、にほん".lastIndexOf("に", 0) == -1);
	assert("こんにちは、にほん".lastIndexOf("あ") == -1);
	assert("こんにちは、にほん".lastIndexOf("日本") == -1);

	assert("こんにちは、にほん".lastIndexOf("に") == 6);
	assert("こんにちは、にほん".indexOf("に") == 2);
	assert("こんにちは、にほん".lastIndexOf("は") == 4);
	assert("こんにちは、にほん".indexOf("は") == 4);
}

test();
