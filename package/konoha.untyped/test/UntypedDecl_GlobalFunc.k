// written by kimio kuramitsu

K.import("konoha.untyped");
K.import("konoha.global");

int succ(int n) {
	return n+1;
}

f = succ;
assert(f(0) == 1);

