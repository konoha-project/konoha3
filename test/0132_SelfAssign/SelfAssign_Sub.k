// written by kimio kuramitsu

import("Syntax.CStyleCompoundAssignment");

void test() {
	int a = 3;
	a -= 1;
	assert(a == 2);
}

test();



