// written by Tetsuro Matsumura

import("js4.array");

void test() {
	int[] a = [1, 2, 9];
	a.clear();
	assert(a.getSize() == 0);
}


test();

