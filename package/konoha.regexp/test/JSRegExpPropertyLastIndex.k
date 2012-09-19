//written by Joseph

import("konoha.string");
import("konoha.regexp");
import("konoha.assign");
import("konoha.class");

void test() {
	String str = "The rain stays.";
	RegExp patt = /ain/g;
	//returns the character position after the last match found exec() or test().
	if (patt.test(str) == true) {
		assert(patt.lastIndex == 8);
	}
}

test();
