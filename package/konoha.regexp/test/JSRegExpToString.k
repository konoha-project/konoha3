//written by Joseph

import("konoha.regexp");
import("konoha.string");
import("konoha.assign");
import("konoha.new");

void test() {
	RegExp myExp = new RegExp("Hello, world!");
	assert(myExp.toString() == "/Hello, world!/");
}

test();
