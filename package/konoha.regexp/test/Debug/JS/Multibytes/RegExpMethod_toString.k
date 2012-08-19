//written by Joseph

K.import("konoha.regexp");
K.import("konoha.string");
K.import("konoha.assign");
K.import("konoha.new");

void test() {
	RegExp myExp = new RegExp("こんにちは、世界");
	assert(myExp.toString() == "/こんにちは、世界/");
}

test();
