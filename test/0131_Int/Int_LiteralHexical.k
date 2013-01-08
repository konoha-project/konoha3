// written by kimio kuramitsu

import("Syntax.CStyleBitwiseOperator"); // konoha.int was obsolete
import("Syntax.Number");

void test() {
	assert(0xdeadbeef == 3735928559);
	assert(0xDEADBEEF == 3735928559);
}

test();

