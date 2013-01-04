//written by Joseph

import("JavaScript.Regexp");

void test() {
	String str = "こんにちは、世界!";
	//look for "こんにちは"
	RegExp patt = /こんにちは/g;
	assert(patt.test(str) == true);
	//look for "日本"
	patt = /日本/g;
	assert(patt.test(str) == false);
}

test();
