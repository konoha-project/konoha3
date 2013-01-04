//written by Joseph

//import("konoha.new"); Don't use this
import("JavaScript.Regexp");

void test() {
	RegExp myExp = new RegExp("こんにちは、世界");
	assert(myExp.toString() == "/こんにちは、世界/");
}

test();
