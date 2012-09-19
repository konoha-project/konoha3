// written by kimio kuramitsu

import("sugar");

Expr tyCheckTrue(Stmt stmt, Expr expr, Gamma gma, int typeid) {
	return stmt.printError("true was redefined");
}

addExprTyCheck("true", tyCheckTrue);
assert(true);


