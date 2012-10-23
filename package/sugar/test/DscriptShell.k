// written by chenji

import("cstyle", "while");
import("konoha.array");
import("konoha.null");
import("posix.path");
import("posix.process");
import("sugar");

String concatTokenList(Token[] tokenList) {
	int i = 0;
	String ret = "";
	while(i < tokenList.getSize()) {
		Token tk = tokenList[i];
		if(tk.isBeforeWhiteSpace()) {
			ret = ret + tk.getText() + " ";
		}
		else {
			ret = ret + tk.getText();
		}
		i = i + 1;
	}
	return ret;
}

boolean statementShell(Stmt stmt, Gamma gma) {
	Token[] tokenList = stmt.getTokenList("$Token", null);
	if(tokenList != null) {
		String cmd = concatTokenList(tokenList);
		System.system(cmd);
		stmt.done();
	}
	return false;
}

boolean isCommand(String cmd) {
	// dummy
	return true;
}

int patternMatchShell(Stmt stmt, int nameid, Token[] tokenList, int beginIdx, int endIdx) {
	Token firstToken = tokenList[beginIdx];
	//System.p("firstToken='" + firstToken.getText() + "', isCommand=" + isCommand(firstToken.getText()));
	if(firstToken.isSymbol() && isCommand(firstToken.getText())) {
		return beginIdx;
	}
	return -1;
}

addPatternMatch("$Shell", patternMatchShell);
addStatement("$Shell", statementShell);
addTopLevelStatement("$Shell", statementShell);

syntax $Shell $Token*

// toplevel test

ls;
ls -la;

void test() {
	ls;
	ls -la;
}

test();
