// written by kimio kuramitsu

import("Type.StaticVar");
import("Syntax.GlobalVariable");

int succ(int n) {
	return n+1;
}

var f = succ;
assert(f(0) == 1);

