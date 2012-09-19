//written by Joseph

import("konoha.string");
import("konoha.array");
import("konoha.assign");

void test() {
	String[] ma = "How are you doing today?".split(" ");
	assert(ma.getSize() == 5);
	assert(ma[0] == "How");
	assert(ma[1] == "are");
	assert(ma[2] == "you");
	assert(ma[3] == "doing");
	assert(ma[4] == "today?");
	ma = "How are you doing today?".split();
	assert(ma.getSize() == 1);
	assert(ma[0] == "How are you doing today?");
	ma = "How are you doing today?".split("o");
	assert(ma.getSize() == 5);
	assert(ma[0] == "H");
	assert(ma[1] == "w are y");
	assert(ma[2] == "u d");
	assert(ma[3] == "ing t");
	assert(ma[4] == "day?");
}

test();
