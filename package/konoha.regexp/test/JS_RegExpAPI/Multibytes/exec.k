//written by Joseph

K.import("konoha.string");
K.import("konoha.regexp");
K.import("konoha.null");
K.import("konoha.assign");

void test() {
	String str = "こんにちは、世界!";
	//look for "こんにちは"
	RegExp patt = /こんにちは/g;
	assert(patt.exec(str) == "こんにちは");
	//look for "日本"
	patt = /日本/g;
	assert(patt.exec(str).isNull());
}

test();
