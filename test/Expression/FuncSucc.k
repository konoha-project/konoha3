// written by kimio kuramitsu

int succ(int n) {
	return n+1;
}

assert(succ(0) == 1);
assert(succ(succ(1)) == 2);
