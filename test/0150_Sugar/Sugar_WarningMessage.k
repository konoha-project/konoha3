// written by kimio kuramitsu

import("konoha.desugar");

Expr TypeCheckTrue(Stmt stmt, Expr expr, Gamma gma, int typeid) {
	return stmt.message(WarnTag, "true was redefined");
}

addTypeCheck("true", TypeCheckTrue);
true;


