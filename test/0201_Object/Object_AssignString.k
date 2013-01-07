// written by kimio kuramitsu

import("JavaStyle.Object");

void test() {
	String s = "123";
	Object o = s;
	assert(o.getTypeId() == s.getTypeId());
}

test();

