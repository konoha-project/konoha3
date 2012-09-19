//written by Joseph

import("konoha.regexp");
import("konoha.assign");
import("konoha.class");

void test() {
	//check whether or not "g" modifier is set.
	RegExp patt = /W3S/g;
	assert(patt.global == true);
	patt = /W3S/;
	assert(patt.global == false);
}

test();
