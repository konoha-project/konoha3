//written by Joseph

K.import("konoha.string");

void test() {
	assert("a".localeCompare("b") == -2);
	assert("b".localeCompare("a") == 2);
	assert("b".localeCompare("b") == 0);
}

test();
