// written by kimio kuramitsu

import("konoha.array");
import("konoha.object");

void test() {
	Object[] a = [1, 2, 3]; // boxing
	assert(a.getSize() == 3);
}

test();

