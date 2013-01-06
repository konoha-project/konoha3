//written by Joseph

import("Java.String");

void test() {
	assert("a".localeCompare("b") == -1);
	assert("b".localeCompare("a") == 1);
	assert("b".localeCompare("b") == 0);
}

test();
