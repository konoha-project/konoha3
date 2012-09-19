// import("sugar")
// import("konoha.while")
// import("konoha.string")
// import("konoha.assign")

// Expr squareBracketParseExpr (Stmt stmt, Array[Token] tokenList, int s, int c,	int e) {
// 	Token tk = tokenList[c];
// 	if(s == c) { // TODO

// 	}
// 	else {
// 		/*kExpr *lexpr = SUGAR kStmt_parseExpr(kctx, stmt, tokenList, s, c);
// 		if(lexpr == K_NULLEXPR) {
// 			RETURN_(lexpr);
// 		}
// 		if(lexpr->syn->keyword == KW_new) {  // new int[100]
// 			kExpr_setsyn(lexpr, SYN_(Stmt_nameSpace(stmt), KW_ExprMethodCall));
// 			lexpr = SUGAR kStmt_addExprParam(kctx, stmt, lexpr, tk->subTokenList, 0, kArray_size(tk->subTokenList), 0/*allowEmpty*/);
// 		}
// 		else {   // X[1] => get X 1
// 			kTokenVar *tkN = GCSAFE_new(TokenVar, 0);
// 			tkN->keyword = MN_toGETTER(0);
// 			tkN->uline = tk->uline;
// 			SugarSyntax *syn = SYN_(Stmt_nameSpace(stmt), KW_ExprMethodCall);
// 			lexpr  = SUGAR new_UntypedCallStyleExpr(kctx, syn, 2, tkN, lexpr);
// 			lexpr = SUGAR kStmt_addExprParam(kctx, stmt, lexpr, tk->subTokenList, 0, kArray_size(tk->subTokenList), 1/*allowEmpty*/);
// 		}
// 		RETURN_(SUGAR kStmt_rightJoinExpr(kctx, stmt, lexpr, tokenList, c+1, e));
// */
// 	}

// 	return 0
// }

// addParseExpr("[]", squareBracketParseExpr);

// []
