//written by Joseph

import("konoha.regexp");
import("konoha.string");
import("konoha.assign");
import("konoha.new");

void test() {
	RegExp myExp = new RegExp("こんにちは、世界");
	assert(myExp.toString() == "/こんにちは、世界/");
}

test();
