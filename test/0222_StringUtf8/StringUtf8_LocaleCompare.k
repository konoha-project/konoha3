//written by Joseph

import("Java.String");

void test() {
	assert("あ".localeCompare("あ") == 0);
	assert("あ".localeCompare("a") == 1);
	assert("あ".localeCompare("0") == 1);
	assert("か".localeCompare("kakiku") == 1);
}

test();
