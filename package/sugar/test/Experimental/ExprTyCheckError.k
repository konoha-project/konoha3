// written by kimio kuramitsu

import("sugar");

Expr tyCheckTrue(Stmt stmt, Expr expr, Gamma gma, int typeid) {
	return stmt.printError("true was redefined");
}

addTypeCheck("true", tyCheckTrue);
assert(true);


