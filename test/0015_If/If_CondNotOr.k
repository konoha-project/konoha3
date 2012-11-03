// written by kimio kuramitsu

int f(int a, int b) {
	if(!(a == 0) || !(b == 1)) {
		return 1;
	}
	else {
		return 0;
	}
}

assert(f(0,0) == 1);
assert(f(0,1) == 0);
assert(f(1,0) == 1);
assert(f(1,1) == 1);

