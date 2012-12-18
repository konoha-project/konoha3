// written by kimio kuramitsu

import("cstyle");

void test() {
	int a = 1;
	a &= 2;
	assert(a == 0);
}

test();



