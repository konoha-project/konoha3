// written by kimio kuramitsu

int f(int a, int b, int c) {
	if(a == 1 && b == 1 && c == 1) {
		return 1;
	}
	else {
		return 0;
	}
}

assert(f(0,0,0) == 0);
assert(f(0,0,1) == 0);
assert(f(0,1,0) == 0);
assert(f(0,1,1) == 0);
assert(f(1,0,0) == 0);
assert(f(1,0,1) == 0);
assert(f(1,1,0) == 0);
assert(f(1,1,1) == 1);

