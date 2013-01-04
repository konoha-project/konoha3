//written by Joseph

import("Syntax.JavaStyleClass");
import("JavaScript.Regexp");

void test() {
	//check whether or not "i" modifier is set.
	RegExp patt = /W3S/i;
	assert(patt.ignoreCase == true);
	patt = /W3S/;
	assert(patt.ignoreCase == false);
}

test();
