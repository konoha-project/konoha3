// written by kimio kuramitsu

import("JavaStyle.Object");

void test() {
	Object o;
	assert(o instanceof Object);
	assert(!(o instanceof int));
	assert(!(o instanceof String));
	assert(!(o instanceof String[]));
}

test();

