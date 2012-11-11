// written by kimio

import("cstyle");

int f(int n) {
	return n;
}

void test() {
	int n = 1;
	assert(f(n++) == 2);  // this must be ERROR
}

test();
