// written by kimio kuramitsu

import("Syntax.GlobalVariable");

int x = 123;

void test() {
	assert(global.x == 123);
}

test();

