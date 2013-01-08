// written by kimio kuramitsu

int add(int a, int b) {
	return a+b;
}

int f(int x, int y) {
	return add({int a = x; a+1;}, {int b = y; b+1;});
}

assert(f(1,1)==4);

