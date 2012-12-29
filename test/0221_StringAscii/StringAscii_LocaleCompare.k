//written by Joseph

import("JavaScript.String");

void test() {
	assert("a".localeCompare("b") == -1);
	assert("b".localeCompare("a") == 1);
	assert("b".localeCompare("b") == 0);
}

test();
