// written by kimio kuramitsu

import("cstyle"); // konoha.null is obsolate; use cstyle

void test () {
	int n, n1 = 1;   // n is 0
	assert(n != null);
	assert(!(n == null));
	assert(n1 != null);
	assert(!(n1 == null));
}

test();
