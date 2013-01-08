// written by kimio kuramitsu

int g(int a, int b) {
	return a+b;
}

int f(int x) {
	return g(1, {int a = x; a+1;});
}

assert(f(1)==3);
