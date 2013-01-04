// written by kimio

import("JavaStyle.Object");

void test() {
	Object o = 123;
	//System.p(o.toString());
	assert(o.toString() == "123");
}
test();
