// written by kimio kuramitsu

int succ(int n) {
	return n+1;
}

assert(succ(0) == 1);
assert(succ(succ(0)) == 2);
