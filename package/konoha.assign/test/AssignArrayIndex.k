// written by kimio kuramitsu

import("konoha.assign");
import("konoha.array");

void test() {
	int[] a = [0];
	a[0] = 10;
	assert(a[0] == 10);
	a[0] = 0;
	assert(a[0] == 0);
}

test();

