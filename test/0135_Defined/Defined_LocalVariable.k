// written by kimio kuramitsu

import("MiniKonoha.NameSpace");  // konoha.defined is obsolete

void test() {
	int a;
	assert(defined(a));
	assert(!defined(b));
}

test();

