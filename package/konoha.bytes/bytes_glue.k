// K.import("sugar");
// K.import("konoha.while");
// K.import("konoha.string");
// K.import("konoha.assignment");

// int singleQuoteToken (Token tk, String source) {
// 	String ch, prev = "'";
// 	int pos = 2;
// 	int len = source.getSize();
// 	System.p(source);
// 	while(pos < len) {
// 		ch = source.get(pos);
// 		if(ch == "\n") {
// 			return 0;
// 		}
// 		if(ch == "'" && prev != "\\") {
// 			p("substr=" + source.substring(1, pos-1));
// 			tk.setText(source.substring(1, pos - 1));
// 			tk.setKeyword("$SingleQuote");
// 			p("returning, " + (pos + 1));
// 			return pos + 1;
// 		}
// 		prev = ch;
// 		pos = pos + 1;
// 	}
// 	return pos-1;
// }

// Expr tyCheckSingleQuote(Stmt stmt, Expr expr, Gamma gma, int typeid) {
// 	Token tk = expr.getTermToken();
// 	String s = tk.getText();
// 	p("test =" +  s);
// 	if (s.getSize() == 1) {
// 		expr.setUnboxConstValue(4, 10);
// 	}
// 	return expr;
// }


// K.addTokenizeFunc("'", singleQuoteToken);
// K.addExprTyCheck("$SingleQuote", tyCheckSingleQuote);
// K.addTermParseExpr("$SingleQuote");
