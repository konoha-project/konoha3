//written by Joseph

//import("konoha.new"); Don't use this
import("JavaScript.Regexp");

void test() {
	RegExp myExp = new RegExp("Hello, world!");
	assert(myExp.toString() == "/Hello, world!/");
}

test();
