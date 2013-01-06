// written by kimio kuramitsu

import("JavaScript.Array"); // if Ok
import("Syntax.CStyleCompoundAssignment");

void test() {
	int[] a = [0, 0];
	assert(a[0] == 0);
	a[0] += 1;   // watch ##
	assert(a[0] == 1);
}

test();



