// written by kimio kuramitsu

import("sugar");

Expr TypeCheckTrue(Stmt stmt, Expr expr, Gamma gma, int typeid) {
	return stmt.printError("true was redefined");
}

addTypeCheck("true", TypeCheckTrue);
assert(true);


