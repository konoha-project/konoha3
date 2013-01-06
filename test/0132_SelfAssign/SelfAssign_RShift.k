// written by kimio kuramitsu

import("Syntax.CStyleCompoundAssignment");
import("Syntax.CStyleBitwiseOperator");

void test() {
	int a = 2;
	a >>= 1;
	assert(a == 1);
}

test();



