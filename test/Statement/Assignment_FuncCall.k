// written by kimio kuramitsu

int succ(int n) {
	return n+1;
}

void test() {
	int a;
	a = succ(0);
	assert(a == 1);
}

test();

