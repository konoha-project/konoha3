// written by kimio kuramitsu

import("konoha.var");
import("konoha.global");

int succ(int n) {
	return n+1;
}

var f = succ;
assert(f(0) == 1);

