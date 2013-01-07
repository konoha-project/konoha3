// written by kimio kuramitsu

import("Syntax.CStyleCompoundAssignment");
import("Syntax.CStyleBitwiseOperator");

void test() {
	int a = 1;
	a |= 2;
	assert(a == 3);
}

test();



