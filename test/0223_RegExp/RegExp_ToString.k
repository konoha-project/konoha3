//written by Joseph

import("konoha.new");
import("konoha.regexp");

void test() {
	RegExp myExp = new RegExp("Hello, world!");
	assert(myExp.toString() == "/Hello, world!/");
}

test();
