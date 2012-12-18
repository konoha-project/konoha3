/****************************************************************************
 * Copyright (c) 2012, the Konoha project authors. All rights reserved.
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  * Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 ***************************************************************************/

import("cstyle", "while");
import("cstyle", "break");
import("cstyle", "null");
import("konoha.array");
import("konoha.console");
import("konoha.ns");
import("konoha.eval");
import("konoha.file");
import("konoha.math");
import("konoha.object");
import("js4.string");
import("dscript.subproc");
import("konoha.desugar");

@Public String FILE.readAll() {
	String ret = "";
	String line = this.readLine();
	while(line != null) {
		ret = ret + line + "\n";
		line = this.readLine();
	}
	if(ret == "") {
		return ret;
	}
	return ret.substr(0, ret.getlength() - 1);
}

String[] concatTokenList(Token[] tokenList) {
	int i = 0;
	String[] ret = [];
	String buf = "";
	while(i < tokenList.getSize()) {
		Token tk = tokenList[i];
		if(tk.isBeforeWhiteSpace()) {
			ret.add(buf + tk.getText());
			buf = "";
		}
		else {
			buf = buf + tk.getText();
		}
		i = i + 1;
	}
	if(buf != "") {
		ret.add(buf);
	}
	return ret;
}

String getVarName() {
	String rand = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
	String ret = "__s";
	while(ret.getlength() < 10) {
		ret = ret + rand[(Math.random() * (rand.getlength() - 1)).toint()];
	}
	return ret;
}

String getFuncName() {
	String rand = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
	String ret = "__f";
	while(ret.getlength() < 10) {
		ret = ret + rand[(Math.random() * (rand.getlength() - 1)).toint()];
	}
	return ret;
}

int indexOf(String[] a, String s, int startIdx) {
	int i = startIdx;
	while(i < a.getSize()) {
		if(a[i] == s) {
			return i;
		}
		i = i + 1;
	}
	return -1;
}

String expandDollar(String arg) {
	if(arg.startsWith("$")) {
		return arg.substr(1) + ".toString()";
	}
	return "\"" + arg + "\"";
}

String expandArgs(String[] args) {
	String ret = "[";
	int i = 0;
	while(i < args.getSize() - 1) {
		ret = ret + expandDollar(args[i]) + ", ";
		i = i + 1;
	}
	if(args.getSize() >= 1) {
		ret = ret + expandDollar(args[args.getSize() - 1]);
	}
	ret = ret + "]";
	return ret;
}

String[] slice(String[] a, int s, int length) {
	String[] ret = [];
	int i = 0;
	while(i < length) {
		ret.add(a[s + i]);
		i = i + 1;
	}
	return ret;
}

boolean isCommandUseTty(String cmd) {
	// TODO: consider using isatty.
	if(cmd == "less" || cmd == "man") {
		return true;
	}
	return false;
}

String join(String[] a, String separator) {
	String ret = "";
	int i = 0;
	while(i < a.getSize() - 1) {
		ret = ret + a[i] + separator;
		i = i + 1;
	}
	if(a.getSize() >= 1) {
		ret = ret + a[a.getSize() - 1];
	}
	return ret;
}

String getPipeString(String prevname, String varname, String[] args, int startindex, int argsize, boolean enableStdout, int index) {
	String ret = "";
	if(prevname != "" && !isCommandUseTty(args[startindex])) {
		ret = ret + prevname + ".pipe(" + varname + ", false);\n";
	}
	if(isCommandUseTty(args[startindex])) {
		if(prevname != "") {
			ret = ret + varname + ".bg();\n";
			if(defined(USE_PRINT)) {
				ret = ret + "System.p(";
			}
			ret = ret + varname + ".communicate(" + prevname + ".getOutputStream().readAll())";
			if(defined(USE_PRINT)) {
				ret = ret + "[0])";
			}
			ret = ret + ";\n";
		}
		else {
			if(enableStdout) {
				ret = ret + varname + ".fg();\n";
			}
			else {
				ret = ret + varname + ".bg();\n";
			}
		}
	}
	else {
		if(enableStdout) {
			if(defined(USE_PRINT) && index < 0) {
				/* use System.p instead of pipe to stdout */
				ret = ret + varname + ".bg();\n";
				ret = ret + "System.p(" + varname + ".communicate(\"\")[0]);\n";
			}
			else {
				ret = ret + varname + ".fg();\n";
			}
		}
		else {
			ret = ret + varname + ".bg();\n";
		}
	}
	return ret;
}

String generateSubProcSequence(String[] args, boolean enableStdout) {
	if(defined(DEBUG)) {
		System.p(args);
	}
	String ret = "";
	int startindex = 0;
	int index = 0;
	String varname = "";
	String prevname = "";
	int argsize = 0;
	int redirectindex = -1;
	String mode = "w";
	while(true) {
		/* pipe */
		index = indexOf(args, "|", startindex);
		if(index < 0) {
			redirectindex = indexOf(args, ">>", startindex);
			if(redirectindex >= 0) {
				mode = "a";
				argsize = redirectindex - startindex - 1;
			}
			else {
				redirectindex = indexOf(args, ">", startindex);
				if(redirectindex >= 0) {
					mode = "w";
					argsize = redirectindex - startindex - 1;
				}
				else {
					argsize = args.getSize() - startindex - 1;
				}
			}
		}
		else {
			argsize = index - startindex - 1;
		}
		varname = getVarName();
		ret = ret + "SubProc " + varname + " = new SubProc(\"" + args[startindex] + "\");\n";
		if(defined(SHOW_COMMAND)) {
			stdout.println(args[startindex]);
		}
		if(argsize > 0) {
			ret = ret + varname + ".setArgumentList(" + expandArgs(slice(args, startindex + 1, argsize)) + ");\n";
		}
		if(index >= 0) {
			ret = ret + getPipeString(prevname, varname, args, startindex, argsize, enableStdout, index);
		}
		else {
			if(redirectindex >= 0) {
				/* redirect */
				String filename = args[redirectindex + 1];
				ret = ret + varname + ".setOutputStream(new FILE(\"" + filename + "\", \"" + mode + "\"));\n";
			}
			else {
				/* end of pipe */
				if(enableStdout && !defined(USE_PRINT)) {
					ret = ret + varname + ".setOutputStream(stdout);\n";
				}
			}
			ret = ret + getPipeString(prevname, varname, args, startindex, argsize, enableStdout, index);
			break;
		}
		prevname = varname;
		startindex = index + 1;
	}
	if(defined(DEBUG)) {
		System.p(ret);
	}
	if(!enableStdout) {
		ret = ret + "return " + varname + ".communicate(\"\")[0]; }";
	}
	return ret;
}

boolean statementShell(Stmt stmt, Gamma gma) {
	Token[] tokenList = stmt.getTokenList("$Token", null);
	if(tokenList.isNull()) {
		return false;
	}
	String[] args = concatTokenList(tokenList);
	Token[] subprocTokens = tokenize(generateSubProcSequence(args, true));
	Block block = stmt.newBlock(subprocTokens, 0, subprocTokens.getSize());
	boolean ret = block.TypeCheckAll(gma);
	if(!ret) {
		return false;
	}
	if(!defined(SHOW_COMMAND)) {
		stmt.setBlock("$Block", block);
	}
	stmt.setType(TSTMT_BLOCK);
	return ret;
}

int patternMatchShell(Stmt stmt, int nameid, Token[] tokenList, int beginIdx, int endIdx) {
	Token firstToken = tokenList[beginIdx];
	if(firstToken.Is("$Symbol") && SubProc.isCommand(firstToken.getText())) {
		int i = beginIdx + 1;
		while(i < endIdx) {
			if(tokenList[i].Is("()")) {
				return -1;
			}
			i = i + 1;
		}
		return beginIdx;
	}
	return -1;
}

Expr expressionAsk(Stmt stmt, Token[] tokenList, int beginIdx, int operatorIdx, int endIdx) {
	if(beginIdx + 1 < endIdx) {
		Token methodToken = new Token("inputUserApproval");
		Expr classExpr = new Expr(Console);
		Expr arg1 = new Expr(tokenList[beginIdx + 1].getText());
		Expr arg2 = new Expr("YES");
		Expr arg3 = new Expr("NO");
		Expr arg4 = new Expr(false);
		Expr expr = stmt.newUntypedCallStyleExpr(methodToken, classExpr, arg1, arg2, arg3, arg4);
		return expr;
	}
}

int tokenFuncCommandReplace(Token tk, String source) {
	int pos = 1;
	int openParenthesis  = 40/*'('*/;
	int closeParenthesis = 41/*')'*/;
	if(source.charCodeAt(pos) != openParenthesis) {
		if(defined(DEBUG)) {
			System.p("skip dollar");
		}
		return 0;
	}
	while(pos < source.getlength()) {
		if(source.charCodeAt(pos) == closeParenthesis) {
			if(tk.isNotNull() && pos > 2) {
				tk.setText("$DollarCommandReplace", source, 2, pos);
			}
			return pos + 1;
		}
		pos = pos + 1;
	}
	if(defined(DEBUG)) {
		System.p("skip dollar");
	}
	return 0;
}

Expr expressionCommandReplace(Stmt stmt, Token[] tokenList, int beginIdx, int operatorIdx, int endIdx) {
	if(defined(DEBUG)) {
		int i = beginIdx;
		while(i < endIdx) {
			System.p(tokenList[i].getText());
			System.p(tokenList[i].getText().getlength());
			System.p(tokenList[i].is("$Symbol"));
			System.p(tokenList[i].is("()"));
			i = i + 1;
		}
	}
	String[] args = tokenList[operatorIdx].getText().trim().split(" ");
	String funcName = getFuncName();
	String evaltext = "@Public String " + funcName + "() { ";
	evaltext = evaltext + generateSubProcSequence(args, false).replace("\n", " ");
	if(defined(DEBUG)) {
		System.p(evaltext);
	}
	eval(evaltext);
	Token methodToken = new Token(funcName);
	Expr classExpr = new Expr(NameSpace);
	Expr expr = stmt.newUntypedCallStyleExpr(methodToken, classExpr);
	return expr;
}

boolean isSymbol(String s) {
	String[] noSymbolList = [" ", "(", ")"];
	int i = 0;
	while(i < noSymbolList.getSize()) {
		if(s.indexOf(noSymbolList[i]) >= 0) {
			return false;
		}
		i = i + 1;
	}
	return true;
}

int tokenFuncEnvironment(Token tk, String source) {
	int pos = 1;
	if(tk.isNull() || !isSymbol(source[pos])) {
		if(defined(DEBUG)) {
			System.p("skip dollar");
		}
		return 0;
	}
	if(source.indexOf("{") == 1) {
		pos = source.indexOf("}");
		if(pos > 2) {
			tk.setText("$DollarEnvironment", source, 2, pos);
			return pos + 1;
		}
		return 0;
	}
	while(pos < source.getlength()) {
		if(!isSymbol(source[pos])) {
			break;
		}
		pos = pos + 1;
	}
	tk.setText("$DollarEnvironment", source, 1, pos);
	return pos;
}

Expr expressionEnvironment(Stmt stmt, Token[] tokenList, int beginIdx, int operatorIdx, int endIdx) {
	if(defined(DEBUG)) {
		int i = beginIdx;
		while(i < endIdx) {
			System.p(tokenList[i].getText());
			System.p(tokenList[i].getText().getlength());
			System.p(tokenList[i].is("$Symbol"));
			System.p(tokenList[i].is("()"));
			i = i + 1;
		}
	}
	Token methodToken = new Token("getenv");
	Expr classExpr = new Expr(System);
	Expr arg1 = new Expr(tokenList[beginIdx].getText());
	Expr expr = stmt.newUntypedCallStyleExpr(methodToken, classExpr, arg1);
	return expr;
}

AddPatternMatch("$Shell", patternMatchShell);
AddStatement("$Shell", statementShell);
AddTopLevelStatement("$Shell", statementShell);
AddExpression("ask", expressionAsk);

syntax $Shell $Token*

AddTokenizer("$DollarCommandReplace", 36/*KonohaChar_Dollar*/, tokenFuncCommandReplace);
AddExpression("$DollarCommandReplace", expressionCommandReplace);

AddTokenizer("$DollarEnvironment", 36/*KonohaChar_Dollar*/, tokenFuncEnvironment);
AddExpression("$DollarEnvironment", expressionEnvironment);
