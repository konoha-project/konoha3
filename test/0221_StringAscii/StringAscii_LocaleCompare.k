//written by Joseph

import("java2.string");

void test() {
	assert("a".localeCompare("b") == -1);
	assert("b".localeCompare("a") == 1);
	assert("b".localeCompare("b") == 0);
}

test();
