//written by Joseph

import("Syntax.JavaStyleClass");
import("JavaScript.Regexp");

void test() {
	//check whether or not "g" modifier is set.
	RegExp patt = /世界/g;
	assert(patt.global == true);
	patt = /世界/;
	assert(patt.global == false);
}

test();
