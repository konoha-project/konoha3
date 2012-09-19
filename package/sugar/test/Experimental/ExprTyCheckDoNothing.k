// written by kimio kuramitsu

import("sugar");

Expr tyCheckTrue(Stmt stmt, Expr expr, Gamma gma, int typeid) {
	System.p("do nothing");
	return expr; // nothing
}

addExprTyCheck("true", tyCheckTrue);

assert(true);


