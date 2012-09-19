// import("sugar");
// import("konoha.null");

// sugar "while" "(" $Expr ")" $Block

// boolean tyCheckWhile(Stmt stmt, Gamma gma) {
//     if(stmt.tyCheckExpr("$Expr", gma, 3/*TY_boolean*/, 0)) {
//         Block block = stmt.getBlock("$Block", null);
//         boolean ret = block.tyCheckAll(gma);
//         stmt.setType(TSTMT_LOOP);
//         return ret;
//     }
//     return false;
// }

// NameSpace.compileAllDefinedMethods();
// addTopStmtTyCheck("while", tyCheckWhile);
// addStmtTyCheck("while", tyCheckWhile);
// addTermParseExpr("while");

