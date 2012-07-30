// written by kimio kuramitsu

K.import("konoha.defined");

int succ(int n) {
	return n+1;
}

assert(defined(succ(int)));
assert(!defined(zero(int)));

