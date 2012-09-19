// written by kimio kuramitsu

import("konoha.assign");

void f(int n) {
	assert(n == 3);
}

void test() {
	int n = 0;
	f(${
		if(n == 0) {
			n = 3;
		}
		n;
	});
}

test();

