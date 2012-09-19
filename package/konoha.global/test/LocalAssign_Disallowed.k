// written by kimio kuramitsu

import("konoha.global");

int x = 123;

void test() {
	assert(x == 123);
	x = 1234;
	assert(x == 1234);
}

test();
assert(x == 1234);


