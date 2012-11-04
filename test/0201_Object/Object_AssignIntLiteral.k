// written by kimio kuramitsu

import("konoha.object");

void test() {
	Object o = 1;
	assert(o.getTypeId() == 4/*TY_int*/);
}

test();

