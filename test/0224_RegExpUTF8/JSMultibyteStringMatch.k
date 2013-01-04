//written by Joseph

import("JavaScript.Array");
import("JavaScript.Regexp");

void test() {
	String[] ma = "世界、こんにちは、世界".match(/世界/g);
	assert(ma.getSize() == 2);
	assert(ma[0] == "世界");
	assert(ma[1] == "世界");
	//assert("世界、こんにちは、世界".match(/せかい/gi).isNull());/*TODO*/
	assert("世界、こんにちは、世界".match(/せかい/gi).getSize() == 0);
	RegExp patt = /[あ-は]/gi;
	ma = "こんにちは、世界abc123!@#".match(patt);
	assert(ma.getSize() == 4);
	assert(ma[0] == "こ");
	assert(ma[1] == "に");
	assert(ma[2] == "ち");
	assert(ma[3] == "は");
	ma = "こんにちは、世界abc123!@#".match(/[0-ん]/g);
	assert(ma.getSize() == 13);
	assert(ma[0] == "こ");
	assert(ma[1] == "ん");
	assert(ma[2] == "に");
	assert(ma[3] == "ち");
	assert(ma[4] == "は");
	assert(ma[5] == "、");
	assert(ma[6] == "a");
	assert(ma[7] == "b");
	assert(ma[8] == "c");
	assert(ma[9] == "1");
	assert(ma[10] == "2");
	assert(ma[11] == "3");
	assert(ma[12] == "@");
}

test();
