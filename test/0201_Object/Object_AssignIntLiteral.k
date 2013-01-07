// written by kimio kuramitsu

import("JavaStyle.Object");

void test() {
	Object o = 1;
	assert(o.getTypeId() == 4/*TY_int*/);
}

test();

