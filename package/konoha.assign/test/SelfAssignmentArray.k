// written by kimio kuramitsu

import("konoha.assign");
import("konoha.array"); // if Ok

void test() {
	int[] a = [0, 0];
	assert(a[0] == 0);
	a[0] += 1;   // watch ##
	assert(a[0] == 1);
}

test();



