//written by Joseph

import("Syntax.JavaStyleClass");
import("JavaScript.Regexp");

void test() {
	RegExp patt = /世界/m;
	assert(patt.multiline == true);
	patt = /世界/;
	assert(patt.multiline == false);
}

test();
