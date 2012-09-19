//written by Joseph

import("konoha.string");
import("konoha.regexp");
import("konoha.assign");
import("konoha.array");
//import("konoha.null");/*TODO*/

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
