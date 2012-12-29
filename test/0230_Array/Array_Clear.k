// written by Tetsuro Matsumura

import("JavaScript.Array");

void test() {
	int[] a = [1, 2, 9];
	a.clear();
	assert(a.getSize() == 0);
}


test();

