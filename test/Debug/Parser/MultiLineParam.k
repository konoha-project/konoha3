// written by kimio

int f(int a, int b) {
	return a+b;
}

int g() {
	return f(1,
	2);
}

assert(g() == 3);

