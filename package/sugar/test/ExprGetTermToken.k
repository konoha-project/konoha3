// written by chenji

import("konoha.while");
import("konoha.string");
import("konoha.null");
import("sugar");

int tokenFuncSingleQuotedChar(Token tk, String source) {
	int ch, prev = 47/*'/'*/, pos = 1;
	while(pos < source.getSize()) {
		ch = source.charCodeAt(pos);
		if(ch == 10/*'\n'*/) {
			break;
		}
		if(ch == 39/*'\''*/ && prev != 92/*'\\'*/) {
			if(tk != null) {
				tk.setText(source.substr(1, pos - 1));
				tk.setUnresolvedTokenType("$SingleQuotedChar");
			}
			return pos + 1;
		}
		prev = ch;
		pos = pos + 1;
	}
	if(tk != null) {
		//FIXME: kreportf
		System.p("must close with \'");
	}
	return 0;
}

Expr typeCheckSingleQuotedChar(Stmt stmt, Expr expr, Gamma gma, int reqty) {
	Token tk = expr.getTermToken();
	String s = tk.getText();
	if(s.getSize() == 1) {
		int ch = s.charCodeAt(0);
		return expr.setUnboxConstValue(4/*TY_int*/, ch);
	}
	else {
		stmt.printError("single quote doesn't accept multi characters, '" + s + "'");
	}
	return null;
}

addTypeCheck("$SingleQuotedChar", typeCheckSingleQuotedChar);
setTokenFunc("$SingleQuotedChar", 17/*KonohaChar_Quote*/, tokenFuncSingleQuotedChar);

void test() {
	int i = 'i';
	assert(i == 105);
}

test();
