// written by kimio kuramitsu

import("sugar");
import("cstyle"); // konoha.null is obsolate; use cstyle

sugar "while" "(" $Expr ")" $Block

boolean TypeCheckWhile(Stmt stmt, Gamma gma) {
	if(stmt.TypeCheckExpr("$Expr", gma, 3/*TY_boolean*/, 0)) {
		return (stmt.getBlock("$Block", null)).TypeCheckAll(gma);
	}
	return false;
}

addTopLevelStatement("while", TypeCheckWhile);
addStatement("while", TypeCheckWhile);

// b = a while (a<10);

void test() {
	while(true) {
		System.p("bye");
	}
}

test();

