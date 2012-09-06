//written by Joseph

K.import("konoha.string");
K.import("konoha.regexp");
K.import("konoha.assign");

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
