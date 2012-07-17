// written by kimio kuramitsu

int succ(int x) {
	return x + 1;
}

int succ(int x, int y) {
	return x + y;
}

void test() {
	assert(succ(1) == 2);
	assert(succ(1,1) == 2);
}

test();

