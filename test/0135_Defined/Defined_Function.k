// written by kimio kuramitsu

import("MiniKonoha.NameSpace");  // konoha.defined is obsolete

int succ(int n) {
	return n+1;
}

assert(defined(succ(int)));
assert(!defined(zero(int)));

