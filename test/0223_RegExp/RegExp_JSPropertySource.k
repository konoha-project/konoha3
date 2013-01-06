//written by Joseph

import("Syntax.JavaStyleClass");
import("JavaScript.Regexp");

void test() {
	RegExp patt = /Anything/g;
	assert(patt.source == "Anything");
}

test();
