// written by kimio kuramitsu

import("JavaStyle.Object");

void test() {
	System sys;
	Object o = sys;
	assert(!(o instanceof boolean));
	assert(!(o instanceof int));
	assert(!(o instanceof String));
	assert((o instanceof System));
	assert(!(o instanceof String[]));
	assert(!(o instanceof Func));
}

test();

