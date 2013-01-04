//written by Joseph

import("Syntax.JavaStyleClass");
import("JavaScript.Regexp");

void test() {
	//check whether or not "i" modifier is set.
	RegExp patt = /世界/i;
	assert(patt.ignoreCase == true);
	patt = /世界/;
	assert(patt.ignoreCase == false);
}

test();
