// written by chenji

import("JavaScript.Array");

void test() {
	int[] ia0 = [0, 1, 2, 3, 4];
	int[] ia1 = ia0.splice(1, 2);
	assert(ia0.getlength() == 3);
	assert(ia0[0] == 0);
	assert(ia0[1] == 3);
	assert(ia0[2] == 4);
	assert(ia1.getlength() == 2);
	assert(ia1[0] == 1);
	assert(ia1[1] == 2);
	int[] ia2 = ia0.splice(1, 2);
	assert(ia0.getlength() == 1);
	assert(ia0[0] == 0);
	assert(ia2.getlength() == 2);
	assert(ia2[0] == 3);
	assert(ia2[1] == 4);

	String[] sa0 = ["a", "b", "c", "d", "e"];
	String[] sa1 = sa0.splice(1, 2);
	assert(sa0.getlength() == 3);
	assert(sa0[0] == "a");
	assert(sa0[1] == "d");
	assert(sa0[2] == "e");
	assert(sa1.getlength() == 2);
	assert(sa1[0] == "b");
	assert(sa1[1] == "c");
	String[] sa2 = sa0.splice(1, 2);
	assert(sa0.getlength() == 1);
	assert(sa0[0] == "a");
	assert(sa2.getlength() == 2);
	assert(sa2[0] == "d");
	assert(sa2[1] == "e");
}

test();
