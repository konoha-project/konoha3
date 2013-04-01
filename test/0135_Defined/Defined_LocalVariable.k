// written by kimio kuramitsu

import("Konoha.NameSpace");  // konoha.defined is obsolete

void test() {
	int a;
	assert(defined(a));
	assert(!defined(b));
}

test();

