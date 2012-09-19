//written by Joseph

import("konoha.string");
import("konoha.array");
import("konoha.assign");

void test() {
	String[] ma = "こんにちは、世界、日本は?".split("、");
	assert(ma.getSize() == 3);
	assert(ma[0] == "こんにちは");
	assert(ma[1] == "世界");
	assert(ma[2] == "日本は?");
	ma = "こんにちは、世界、日本は?".split();
	assert(ma.getSize() == 1);
	assert(ma[0] == "こんにちは、世界、日本は?");
	ma = "こんにちは、世界、日本は?".split("は");
	assert(ma.getSize() == 3);
	assert(ma[0] == "こんにち");
	assert(ma[1] == "、世界、日本");
	assert(ma[2] == "?");
}

test();
