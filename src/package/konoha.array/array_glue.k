// import("konoha.desugar")
// import("konoha.while")
// import("java2.string")
// import("konoha.assign")

// Expr squareBracketExpression (Stmt stmt, Array[Token] tokenList, int s, int c,	int e) {
// 	Token tk = tokenList[c];
// 	if(s == c) { // TODO

// 	}
// 	else {
// 		/*kExpr *lexpr = SUGAR kStmt_ParseExpr(kctx, stmt, tokenList, s, c);
// 		if(lexpr == K_NULLEXPR) {
// 			KReturn(lexpr);
// 		}
// 		if(lexpr->syn->keyword == KW_new) {  // new int[100]
// 			kExpr_Setsyn(lexpr, SYN_(Stmt_ns(stmt), KW_ExprMethodCall));
// 			lexpr = SUGAR kStmt_AddExprParam(kctx, stmt, lexpr, tk->subTokenList, 0, kArray_size(tk->subTokenList), 0/*allowEmpty*/);
// 		}
// 		else {   // X[1] => get X 1
// 			kTokenVar *tkN = /*G*/new_(TokenVar, 0);
// 			tkN->keyword = MN_toGETTER(0);
// 			tkN->uline = tk->uline;
// 			SugarSyntax *syn = SYN_(Stmt_ns(stmt), KW_ExprMethodCall);
// 			lexpr  = SUGAR new_UntypedCallStyleExpr(kctx, syn, 2, tkN, lexpr);
// 			lexpr = SUGAR kStmt_AddExprParam(kctx, stmt, lexpr, tk->subTokenList, 0, kArray_size(tk->subTokenList), 1/*allowEmpty*/);
// 		}
// 		KReturn(SUGAR kStmt_RightJoinExpr(kctx, stmt, lexpr, tokenList, c+1, e));
// */
// 	}

// 	return 0
// }

// addExpression("[]", squareBracketExpression);

// []
