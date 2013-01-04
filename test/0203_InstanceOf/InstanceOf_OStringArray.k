// written by kimio kuramitsu

import("JavaStyle.Object");

void test() {
	String[] a;
	Object o = a;
	assert(!(o instanceof boolean));
	assert(!(o instanceof int));
	assert(!(o instanceof String));
	assert(!(o instanceof System));
	assert((o instanceof String[]));
	assert(!(o instanceof Func));
}

test();

