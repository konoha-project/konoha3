//written by Joseph

K.import("konoha.string");
K.import("konoha.regexp");
K.import("konoha.null");
K.import("konoha.assign");

void test() {
	String str = "Hello world!";
	//look for "Hello"
	RegExp patt = /Hello/g;
	assert(patt.exec(str) == "Hello");
	//look for "W3Schools"
	patt = /W3Schools/g;
	assert(patt.exec(str).isNull());
}

test();
