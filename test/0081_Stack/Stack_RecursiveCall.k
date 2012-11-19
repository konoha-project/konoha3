// written by kimio

int fact(int n) {
	if(n == 0) {
		return 1;
	}
	return n * fact(n-1);
}

assert(fact(4) == 24);

