//written by Joseph

import("Syntax.JavaStyleClass");
import("JavaScript.Regexp");

void test() {
	String str = "The rain stays.";
	RegExp patt = /ain/g;
	//returns the character position after the last match found exec() or test().
	if(patt.test(str) == true) {
		assert(patt.lastIndex == 8);
	}
}

test();
