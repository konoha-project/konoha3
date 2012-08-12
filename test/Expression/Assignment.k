// written by kimio kuramitsu

int succ(int n) {
	return n+1;
}

void test() {
	int a;
	assert(succ(a = 1) == 2);
	assert(a == 1);
}

test();

