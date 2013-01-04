// written by chenji

import("JavaScript.Array");

void test() {
	int[] ia0 = [0, 1, 2, 3, 4];
	int[] ia1 = ia0.slice(2);
	int[] ia2 = ia0.slice(2, 3);
	assert(ia0.getlength() == 5);
	assert(ia1.getlength() == 3);
	assert(ia1[0] == 2);
	assert(ia1[1] == 3);
	assert(ia1[2] == 4);
	assert(ia2.getlength() == 1);
	assert(ia2[0] == 2);

	String[] sa0 = ["a", "b", "c", "d", "e"];
	String[] sa1 = sa0.slice(2);
	String[] sa2 = sa0.slice(2, 3);
	assert(sa0.getlength() == 5);
	assert(sa1.getlength() == 3);
	assert(sa1[0] == "c");
	assert(sa1[1] == "d");
	assert(sa1[2] == "e");
	assert(sa2.getlength() == 1);
	assert(sa2[0] == "c");
}

test();
