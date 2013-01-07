// written by kimio kuramitsu

import("JavaScript.Array");
import("JavaStyle.Object");

void test() {
	Object[] a = [1, 2, 3]; // boxing
	assert(a.getSize() == 3);
}

test();

