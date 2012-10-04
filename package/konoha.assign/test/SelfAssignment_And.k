// written by kimio kuramitsu

import("konoha.int");
import("konoha.assign");

void test() {
	int a = 1;
	a &= 2;
	assert(a == 0);
}

test();



