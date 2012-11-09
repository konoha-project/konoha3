//written by Joseph

import("konoha.class");
import("konoha.regexp");

void test() {
	//check whether or not "g" modifier is set.
	RegExp patt = /世界/g;
	assert(patt.global == true);
	patt = /世界/;
	assert(patt.global == false);
}

test();
