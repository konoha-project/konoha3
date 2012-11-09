//written by Joseph

import("konoha.class");
import("konoha.regexp");

void test() {
	//check whether or not "i" modifier is set.
	RegExp patt = /世界/i;
	assert(patt.ignoreCase == true);
	patt = /世界/;
	assert(patt.ignoreCase == false);
}

test();
