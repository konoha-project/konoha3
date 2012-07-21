// K.import("sugar")
// K.import("konoha.while")
// K.import("konoha.string")
// K.import("konoha.assignment")

// Expr squareBracketParseExpr (Stmt stmt, Array[Token] tokenArray, int s, int c,	int e) {
// 	Token tk = tokenArray[c];
// 	if(s == c) { // TODO

// 	}
// 	else {
// 		/*kExpr *lexpr = SUGAR kStmt_parseExpr(kctx, stmt, tokenArray, s, c);
// 		if(lexpr == K_NULLEXPR) {
// 			RETURN_(lexpr);
// 		}
// 		if(lexpr->syn->keyword == KW_new) {  // new int[100]
// 			kExpr_setsyn(lexpr, SYN_(Stmt_nameSpace(stmt), KW_ExprMethodCall));
// 			lexpr = SUGAR kStmt_addExprParam(kctx, stmt, lexpr, tk->sub, 0, kArray_size(tk->sub), 0/*allowEmpty*/);
// 		}
// 		else {   // X[1] => get X 1
// 			kTokenVar *tkN = GCSAFE_new(TokenVar, 0);
// 			tkN->keyword = MN_toGETTER(0);
// 			tkN->uline = tk->uline;
// 			SugarSyntax *syn = SYN_(Stmt_nameSpace(stmt), KW_ExprMethodCall);
// 			lexpr  = SUGAR new_ConsExpr(kctx, syn, 2, tkN, lexpr);
// 			lexpr = SUGAR kStmt_addExprParam(kctx, stmt, lexpr, tk->sub, 0, kArray_size(tk->sub), 1/*allowEmpty*/);
// 		}
// 		RETURN_(SUGAR kStmt_rightJoinExpr(kctx, stmt, lexpr, tokenArray, c+1, e));
// */
// 	}

// 	return 0
// }

// K.addParseExpr("[]", squareBracketParseExpr);

// []
