// written by kimio kuramitsu

import("JavaStyle.Object");

void test() {
	Object o = "hi";
	assert(!(o instanceof boolean));
	assert(!(o instanceof int));
	assert((o instanceof String));
	assert(!(o instanceof System));
	assert(!(o instanceof String[]));
	assert(!(o instanceof Func));
}

test();

