// written by kimio kuramitsu

import("Syntax.CStyleCompoundAssignment");

void test() {
	int a = 6;
	a %= 4;
	assert(a == 2);
}

test();



