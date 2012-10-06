// written by kimio kuramitsu

import("sugar");
import("konoha.null");

sugar "while" "(" $Expr ")" $Block

boolean tyCheckWhile(Stmt stmt, Gamma gma) {
	if(stmt.tyCheckExpr("$Expr", gma, 3/*TY_boolean*/, 0)) {
		return (stmt.getBlock("$Block", null)).tyCheckAll(gma);
	}
	return false;
}

addTopLevelStatement("while", tyCheckWhile);
addStatement("while", tyCheckWhile);

// b = a while (a<10);

void test() {
	while(true) {
		System.p("bye");
	}
}

test();

