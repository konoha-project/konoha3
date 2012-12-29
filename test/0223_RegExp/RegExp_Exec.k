//written by Joseph

import("JavaScript.Array");
import("JavaScript.Regexp");

void test() {
	String str = "Hello world!";
	//look for "Hello"
	RegExp patt = /Hello/g;
	String[] ma = patt.exec(str);
	assert(ma.getSize() == 1);
	assert(ma[0] == "Hello");
	//look for "W3Schools"
	patt = /W3Schools/g;
	//assert(patt.exec(str).isNull());/*TODO*/
	assert(patt.exec(str).getSize() == 0);
}

test();
