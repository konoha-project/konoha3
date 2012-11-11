// written by kimio kuramitsu

import("konoha.var");

int succ(int n) {
	return n+1;
}

void test() {
	var f = succ;
	assert(f(0) == 1);
}

test();


