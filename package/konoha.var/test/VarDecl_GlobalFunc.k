// written by kimio kuramitsu

K.import("konoha.var");
K.import("konoha.global");

int succ(int n) {
	return n+1;
}

var f = succ;
assert(f(0) == 1);

