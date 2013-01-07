// import("MiniKonoha.Syntax");
// import("cstyle"); // use cstyle
// import("Java.String");

// int singleQuoteToken (Token tk, String source) {
// 	String ch, prev = "'";
// 	int pos = 2;
// 	int len = source.getSize();
// 	while(pos < len) {
// 		ch = source.get(pos);
// 		if(ch == "\n") {
// 			return 0;
// 		}
// 		if(ch == "'" && prev != "\\") {
// 			tk.setText(source.substring(1, pos - 1));
// 			tk.setKeyword("$SingleQuotedChar");
// 			return pos + 1;
// 		}
// 		prev = ch;
// 		pos = pos + 1;
// 	}
// 	return pos-1;
// }

// Expr TypeCheckSingleQuotedChar(Stmt stmt, Expr expr, Gamma gma, int typeid) {
// 	Token tk = expr.getTermToken();
// 	String s = tk.getText();
// 	if(s.getSize() == 1) {
// 		expr.setUnboxConstValue(4, s.charCodeAt(0));
// 	} else {
// 		stmt.printError("single quote doesn't accept multi characters, '" + s + "'");
// 	}
// 	return expr;
// }


// setTokenFunc("'", singleQuoteToken);
// addTypeCheck("$SingleQuotedChar", TypeCheckSingleQuotedChar);
// addTermExpression("$SingleQuotedChar");
