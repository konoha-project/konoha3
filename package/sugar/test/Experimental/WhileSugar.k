// written by kimio kuramitsu

K.import("sugar");
K.import("konoha.null");

sugar "while" "(" $Expr ")" $Block

boolean tyCheckWhile(Stmt stmt, Gamma gma) {
	if(stmt.tyCheckExpr("$Expr", gma, 3/*TY_Boolean*/, 0)) {
		return (stmt.getBlock("$Block", null)).tyCheckAll(gma);
	}
	return false;
}

K.addTopStmtTyCheck("while", tyCheckWhile);
K.addStmtTyCheck("while", tyCheckWhile);

// b = a while (a<10);

void test() {
	while(true) {
		System.p("bye");
	}
}

test();

