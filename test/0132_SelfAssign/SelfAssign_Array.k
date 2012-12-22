// written by kimio kuramitsu

import("js4.array"); // if Ok
import("cstyle");

void test() {
	int[] a = [0, 0];
	assert(a[0] == 0);
	a[0] += 1;   // watch ##
	assert(a[0] == 1);
}

test();



