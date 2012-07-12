
/**
K.import("sugar");

@Public boolean Stmt.tyCheckWhile(Gamma gma) {
	if(tyCheckExpr("$expr", gma, TY_Boolean, 0)) {
		return (Stmt.getBlock("$block", null)).tyCheckAll(gma);
	}
	return false;
}
**/
