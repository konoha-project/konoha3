//written by Joseph

K.import("konoha.string");
K.import("konoha.regexp");
K.import("konoha.assign");
K.import("konoha.class");

void test() {
	String str = "こんにちは、世界!";
	RegExp patt = /世界/g;
	//returns the character position after the last match found exec() or test().
	if (patt.test(str) == true) {
		assert(patt.lastIndex == 8);
	}
}

test();
