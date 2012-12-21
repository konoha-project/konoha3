// written by shinpei

import("js4.array")

void test() {
	int[] ia = [1,2,3];
	assert(ia.indexOf(1) == 0);
	assert(ia.indexOf(2) == 1);
	assert(ia.indexOf(3) == 2);
	assert(ia.indexOf(5) == -1);

}

test();
