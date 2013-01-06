// written by shinpei

import("JavaScript.Array")

void test() {
	String[] ia = ["a","b","c"];
	assert(ia.indexOf("a") == 0);
	assert(ia.indexOf("b") == 1);
	assert(ia.indexOf("c") == 2);
	assert(ia.indexOf("d") == -1);

}

test();
