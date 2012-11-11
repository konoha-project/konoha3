// written by kimio kuramitsu

import("konoha.untyped");

int succ(int n) {
	return n+1;
}

void test() {
	f = succ;
	assert(f(0) == 1);
}

test();


