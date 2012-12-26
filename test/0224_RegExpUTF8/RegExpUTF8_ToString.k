//written by Joseph

import("konoha.new");
import("js4.regexp");

void test() {
	RegExp myExp = new RegExp("こんにちは、世界");
	assert(myExp.toString() == "/こんにちは、世界/");
}

test();
