// written by kimio kuramitsu

import("JavaStyle.Object");

void test() {
	Object o = "123";
	assert(o.getTypeId() == ("123").getTypeId());
}

test();

