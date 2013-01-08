// written by kimio kuramitsu

int g(int a, int b) {
	return a+b;
}

int f(int x) {
	return g({int a = x; a+1;}, 3);
}

assert(f(1)==5);
