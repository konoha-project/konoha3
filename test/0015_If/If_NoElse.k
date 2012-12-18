// written by kimio kuramitsu

void f(int n) {
	if(n > 0) {
		assert(n > 0);
		return;
	}
	assert(n <= 0);
}

f(0);
f(1);


