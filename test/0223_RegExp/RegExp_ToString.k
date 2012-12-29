//written by Joseph

import("konoha.new");
import("JavaScript.Regexp");

void test() {
	RegExp myExp = new RegExp("Hello, world!");
	assert(myExp.toString() == "/Hello, world!/");
}

test();
