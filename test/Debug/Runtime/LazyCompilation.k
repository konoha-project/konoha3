// written by kimio

int f(int n) {
	return g(n);
}

int g(int n) {
	return n+1;
}

assert(f(1) == 2);

