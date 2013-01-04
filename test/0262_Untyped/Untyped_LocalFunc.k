// written by kimio kuramitsu

import("Syntax.UntypedVariable");

int succ(int n) {
	return n+1;
}

void test() {
	f = succ;
	assert(f(0) == 1);
}

test();


