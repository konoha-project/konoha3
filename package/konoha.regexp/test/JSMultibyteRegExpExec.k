//written by Joseph

import("konoha.regexp");
//import("konoha.null");/*TODO*/
import("konoha.assign");
import("konoha.array");
import("konoha.class");

void test() {
	String str = "こんにちは、世界!";
	//look for "こんにちは"
	RegExp patt = /こんにちは/g;
	String[] ma = patt.exec(str);
	assert(ma.getSize() == 1);
	assert(ma[0] == "こんにちは");
	//look for "日本"
	patt = /日本/g;
	//assert(patt.exec(str).isNull());/*TODO*/
	assert(patt.exec(str).getSize() == 0);
}

test();
