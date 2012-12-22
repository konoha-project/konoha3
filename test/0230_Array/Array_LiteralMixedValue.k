// written by kimio kuramitsu

import("js4.array");
import("konoha.object");

void test() {
	Object[] a = [1, "2", true]; // boxing
	assert(a.getSize() == 3);
}

test();

