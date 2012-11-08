//written by Joseph

import("konoha.string");

void test() {
	assert("\b".charCodeAt(0) == 8);
	assert("\t".charCodeAt(0) == 9);
	assert("\n".charCodeAt(0) == 10);
	assert("\v".charCodeAt(0) == 11);
	assert("\f".charCodeAt(0) == 12);
	assert("\r".charCodeAt(0) == 13);
	assert("\"".charCodeAt(0) == 34);
	assert("\'".charCodeAt(0) == 39);
	assert("\\".charCodeAt(0) == 92);
	//assert("\x65".charCodeAt(0) == 101);
	//assert("\uFF21".charCodeAt(0) == 65313);
}

test();
