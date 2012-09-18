// written by kimio kuramitsu
int f() {
	int a = 1, b = 2;
	assert(a == 1);
	assert(b == 2);
	return a+b;
}

assert(f() == 3);

