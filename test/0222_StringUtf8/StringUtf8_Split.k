//written by Joseph

import("JavaScript.Array");
import("JavaScript.String");

void test() {
	String[] ma = "こんにちは、世界、日本は?".split("、");
	assert(ma.getlength() == 3);
	assert(ma[0] == "こんにちは");
	assert(ma[1] == "世界");
	assert(ma[2] == "日本は?");
	ma = "こんにちは、世界、日本は?".split();
	assert(ma.getlength() == 1);
	assert(ma[0] == "こんにちは、世界、日本は?");
	ma = "こんにちは、世界、日本は?".split("は");
	assert(ma.getlength() == 3);
	assert(ma[0] == "こんにち");
	assert(ma[1] == "、世界、日本");
	assert(ma[2] == "?");
}

test();

void test1() {
    String[] a = ("あい".split(""));
    assert(a[0] == "あ");
    assert(a[1] == "い");
    String[] b = ("あいueoうえお".split(""));
    assert(b[0] == "あ");
    assert(b[1] == "い");
    assert(b[2] == "u");
    assert(b[3] == "e");
    assert(b[4] == "o");
    assert(b[5] == "う");
    assert(b[6] == "え");
    assert(b[7] == "お");
}

test1();
