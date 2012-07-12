// written by kimio kuramitsu

K.import("sugar");

Expr tyCheckTrue(Stmt stmt, Expr expr, Gamma gma, int typeid) {
	System.p("do nothing");
	return expr; // nothing
}

K.addExprTyCheck("true", tyCheckTrue);
assert(true);


