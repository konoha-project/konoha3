// written by kimio kuramitsu

import("konoha.object");

void test() {
	Object o = "123";
	assert(o.getTypeId() == ("123").getTypeId());
}

test();

