// written by kimio kuramitsu

import("Syntax.Null");

void test () {
	int n, n1 = 1;   // n is 0
	assert(n != null);
	assert(!(n == null));
	assert(n1 != null);
	assert(!(n1 == null));
}

test();
