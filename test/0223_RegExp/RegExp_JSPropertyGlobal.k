//written by Joseph

import("konoha.class");
import("konoha.regexp");

void test() {
	//check whether or not "g" modifier is set.
	RegExp patt = /W3S/g;
	assert(patt.global == true);
	patt = /W3S/;
	assert(patt.global == false);
}

test();
