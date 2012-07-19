K.import("sugar");
K.import("konoha.while");
K.import("konoha.string");
K.import("konoha.assignment");

int singleQuoteToken (Token tk, String source) {
	String ch, prev = "'";
	int pos = 2;
	int len = source.getSize();
	while(pos < len) {
		ch = source.get(pos);
		if(ch == "\n") {
			return 0;
		}
		if(ch == "'" && prev != "\\") {
			tk.setText(source.substring(1, pos - 1));
			tk.setKeyword("$SingleQuote");
			return pos + 1;
		}
		prev = ch;
		pos = pos + 1;
	}
	return pos-1;
}

Expr tyCheckSingleQuote(Stmt stmt, Expr expr, Gamma gma, int typeid) {
	Token tk = expr.getTermToken();
	String s = tk.getText();
	if (s.getSize() == 1) {
		expr.setUnboxConstValue(4, s.charCodeAt(0));
	} else {
		stmt.printError("single quote doesn't accept multi characters, '" + s + "'");
	}
	return expr;
}


K.addTokenizeFunc("'", singleQuoteToken);
K.addExprTyCheck("$SingleQuote", tyCheckSingleQuote);
K.addTermParseExpr("$SingleQuote");
