// written by chenji

import("JavaScript.Array");

void test() {
	int[] ia = [1, 2, 3];
	assert(ia.join()    == "1,2,3");
	assert(ia.join(" ") == "1 2 3");
	String[] sa = ["a", "b", "c"];
	assert(sa.join()    == "\"a\",\"b\",\"c\"");
	assert(sa.join(" ") == "\"a\" \"b\" \"c\"");
}

test();
