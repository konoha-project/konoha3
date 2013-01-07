// written by kimio kuramitsu

void f(int n) {
	System.p("n="+n);
	assert(n == 3);
}

void test() {
	int n = 0;
	f(${
		if(n == 0) {
			n = 3;
		}
	});
}

test();

