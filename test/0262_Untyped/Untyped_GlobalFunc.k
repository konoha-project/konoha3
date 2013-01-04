// written by kimio kuramitsu

import("Syntax.UntypedVariable");
import("Syntax.GlobalVariable");

int succ(int n) {
	return n+1;
}

f = succ;
assert(f(0) == 1);

