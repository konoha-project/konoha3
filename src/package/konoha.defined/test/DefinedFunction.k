// written by kimio kuramitsu

import("konoha.defined");

int succ(int n) {
	return n+1;
}

assert(defined(succ(int)));
assert(!defined(zero(int)));

