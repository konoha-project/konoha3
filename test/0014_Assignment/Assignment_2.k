// written by kimio kuramitsu

int succ(int n) {
	return n+1;
}

void test() {
	int a;
	int b = succ(a = 1);
	assert(a == 1);
	assert(b == 2);
}

test();

