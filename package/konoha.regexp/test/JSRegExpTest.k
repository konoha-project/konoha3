//written by Joseph

import("konoha.string");
import("konoha.regexp");
import("konoha.assign");

void test() {
	String str = "Hello world!";
	//look for "Hello"
	RegExp patt = /Hello/g;
	assert(patt.test(str) == true);
	//look for "W3Schools"
	patt = /W3Schools/g;
	assert(patt.test(str) == false);
}

test();
