// written by kimio kuramitsu

import("konoha.global");

int x = 123;

void test() {
	assert(global.x == 123);
	global.x = 1234;
	assert(global.x == 1234);
}

test();
assert(x == 1234);


