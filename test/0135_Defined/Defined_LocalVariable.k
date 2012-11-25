// written by kimio kuramitsu

import("konoha.defined");

void test() {
	int a;
	assert(defined(a));
	assert(!defined(b));
}

test();

