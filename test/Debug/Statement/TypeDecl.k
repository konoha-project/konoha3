// written by kimio kuramitsu
int f(int n) {
	int a = 1;
	assert(a == 1);
	return a+n;
}

assert(f(0) == 1);
assert(f(1) == 2);

