// written by kimio kuramitsu

import("konoha.namespace");  // konoha.defined is obsolete

void test() {
	int a;
	assert(defined(a));
	assert(!defined(b));
}

test();

