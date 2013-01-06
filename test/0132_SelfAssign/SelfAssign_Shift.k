// written by kimio kuramitsu

import("Syntax.CStyleCompoundAssignment");
import("Syntax.CStyleBitwiseOperator");

void test() {
	int a = 1;
	a <<= 1;
	assert(a == 2);
}

test();



