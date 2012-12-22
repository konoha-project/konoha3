// written by chenji

import("js4.array");

void test() {
	int[] ia = [1, 2, 3];
	assert(ia.join()    == "1,2,3");
	assert(ia.join(" ") == "1 2 3");
	String[] sa = ["a", "b", "c"];
	assert(sa.join()    == "\"a\",\"b\",\"c\"");
	assert(sa.join(" ") == "\"a\" \"b\" \"c\"");
}

test();
