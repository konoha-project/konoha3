// written by kimio kuramitsu

import("konoha.int");
// import("konoha.assign"); Don't use any more

void test() {
	int a = 1;
	a &= 2;
	assert(a == 0);
}

test();



