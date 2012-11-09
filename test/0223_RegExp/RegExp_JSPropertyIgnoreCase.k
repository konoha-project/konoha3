//written by Joseph

import("konoha.class");
import("konoha.regexp");

void test() {
	//check whether or not "i" modifier is set.
	RegExp patt = /W3S/i;
	assert(patt.ignoreCase == true);
	patt = /W3S/;
	assert(patt.ignoreCase == false);
}

test();
