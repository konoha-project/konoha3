// written by shinpei

import("konoha.assign");
import("konoha.array");

void test () {
	String[] sa1 = ["a", "b"];
	String[] sa2 = ["c", "d", "e"];

	String[] sa = sa1.concat(sa2);
	assert(sa.getSize() == 5);
	assert(sa[0] == "a");
	assert(sa[1] == "b");
	assert(sa[2] == "c");
	assert(sa[3] == "d");
	assert(sa[4] == "e");
}

test();
