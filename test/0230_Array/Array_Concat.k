// written by chenji

import("JavaScript.Array");

void test() {
	int[] ia0 = [0, 1, 2];
	int[] ia1 = [3, 4];
	int[] ia2 = ia0.concat(ia1);
	assert(ia0.getlength() == 3);
	assert(ia1.getlength() == 2);
	assert(ia2.getlength() == 5);
	assert(ia2[0] == 0);
	assert(ia2[1] == 1);
	assert(ia2[2] == 2);
	assert(ia2[3] == 3);
	assert(ia2[4] == 4);

	String[] sa0 = ["a", "b", "c"];
	String[] sa1 = ["d", "e"];
	String[] sa2 = sa0.concat(sa1);
	assert(sa0.getlength() == 3);
	assert(sa1.getlength() == 2);
	assert(sa2.getlength() == 5);
	assert(sa2[0] == "a");
	assert(sa2[1] == "b");
	assert(sa2[2] == "c");
	assert(sa2[3] == "d");
	assert(sa2[4] == "e");
}

test();
