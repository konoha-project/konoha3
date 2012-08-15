//written by Joseph

K.import("konoha.regexp");
K.import("konoha.string");
K.import("konoha.assign");

void test() {
	RegExp myExp = new RegExp("Hello, world!");
	assert(myExp.toString() == "Hello, world!");
}

test();
