// written by kimio kuramitsu

import("konoha.untyped");
import("konoha.global");

int succ(int n) {
	return n+1;
}

f = succ;
assert(f(0) == 1);

