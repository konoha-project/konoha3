//K.import("sugar");
//K.import("konoha.null");
//
//sugar "while" "(" $expr ")" $block
//
//boolean tyCheckWhile(Stmt stmt, Gamma gma) {
//    if(stmt.tyCheckExpr("$expr", gma, 3/*TY_Boolean*/, 0)) {
//        Block block = stmt.getBlock("$block", null);
//        boolean ret = block.tyCheckAll(gma);
//        stmt.setType(TSTMT_LOOP);
//        return ret;
//    }
//    return false;
//}
//
//NameSpace.compileAllDefinedMethods();
//K.addTopStmtTyCheck("while", tyCheckWhile);
//K.addStmtTyCheck("while", tyCheckWhile);
