// written by kimio kuramitsu

import("JavaStyle.Object");

void test() {
	int n = 123;
	Object o = n;
	assert(o.getTypeId() == n.getTypeId());
}

test();

