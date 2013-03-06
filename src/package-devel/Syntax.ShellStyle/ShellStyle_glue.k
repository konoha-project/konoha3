/****************************************************************************
 * Copyright (c) 2013, the Konoha project authors. All rights reserved.
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

Import("dscript.subproc");
Import("JavaScript.Array");
Import("JavaScript.Math");
Import("JavaScript.String");
Import("MiniKonoha.Eval");
Import("MiniKonoha.NameSpace");
Import("MiniKonoha.Syntax");
Import("posix.path");
Import("Syntax.CStyleWhile");
Import("Syntax.Null");
Import("Type.File");

@Override @Public boolean String.startsWith(String str) {
	return StringUtil.startsWith(this, str, 0);
}

@Override @Public boolean String.endsWith(String str) {
	return StringUtil.endsWith(this, str);
}

String[] concatTokenList(Token[] tokenList) {
	if(defined(DEBUG)) {
		System.p(tokenList);
	}
	int i = 0;
	String[] ret = [];
	String buf = "";
	while(i < tokenList.getlength()) {
		Token tk = tokenList[i];
		if(tk.isBeforeWhiteSpace()) {
			ret.add(buf + tk.getParsedText());
			buf = "";
		}
		else if(tk.Is("$Member")) {
			buf = buf + "." + tk.getParsedText();
		}
		else {
			buf = buf + tk.getParsedText();
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

int indexOf(String[] a, String s, int startIdx) {
	int i = startIdx;
	while(i < a.getlength()) {
		if(a[i] == s) {
			return i;
		}
		i = i + 1;
	}
	return -1;
}

String expandDollar(String arg) {
	if(arg.startsWith("\"") && arg.endsWith("\"")) {
		return arg;
	}
	return "\"" + arg + "\"";
}

String expandArgs(String[] args) {
	String ret = "[";
	int i = 0;
	while(i < args.getlength() - 1) {
		ret = ret + expandDollar(args[i]) + ", ";
		i = i + 1;
	}
	if(args.getlength() >= 1) {
		ret = ret + expandDollar(args[args.getlength() - 1]);
	}
	ret = ret + "]";
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
	while(i < a.getlength() - 1) {
		ret = ret + a[i] + separator;
		i = i + 1;
	}
	if(a.getlength() >= 1) {
		ret = ret + a[a.getlength() - 1];
	}
	return ret;
}

String getPipeString(String prevname, String varname, String[] args, int startindex, int argsize, boolean enableStdout, int index, boolean ignoreErrorStatus) {
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
	if(!ignoreErrorStatus) {
		//ret = ret + "if(" + varname + ".getStatus() != 0) { exit(" + varname + ".getStatus()); }\n";
		ret = ret + "assert(" + varname + ".getStatus() == 0);\n";
	}
	ret = ret + "global.lastExitStatus = " + varname + ".getStatus();\n";
	return ret;
}

String generateSubProcSequence(String[] args, boolean enableStdout, boolean ignoreErrorStatus) {
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
					argsize = args.getlength() - startindex - 1;
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
			ret = ret + varname + ".setArgumentList(" + expandArgs(args.slice(startindex + 1, startindex + 1 + argsize)) + ");\n";
		}
		if(index >= 0) {
			ret = ret + getPipeString(prevname, varname, args, startindex, argsize, enableStdout, index, ignoreErrorStatus);
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
			ret = ret + getPipeString(prevname, varname, args, startindex, argsize, enableStdout, index, ignoreErrorStatus);
			break;
		}
		prevname = varname;
		startindex = index + 1;
	}
	if(!enableStdout) {
		ret = ret + varname + ".communicate(\"\")[0];\n";
	}
	if(defined(DEBUG)) {
		System.p(ret);
	}
	return ret;
}

Node TypeShell(Node node, NameSpace ns, Object o) {
	Node sequence = node.TypeCheckNode("$SubprocSequence", o, TypeCheckPolicy_AllowEmpty);
	return sequence;
}

int ParseShell(Node node, Symbol sym, Token[] tokenList, int beginIdx, int operatorIdx, int endIdx) {
	Token firstToken = tokenList[beginIdx];
	boolean ignoreErrorStatus = false;
	boolean isSymbol = false;
	if(firstToken.Is("!")) {
		ignoreErrorStatus = true;
		firstToken = tokenList[beginIdx + 1];
		beginIdx = beginIdx + 1;
	}
	if(firstToken.getParsedText() == "/") {
		int i = beginIdx + 1;
		String cmd = "/";
		while(i < endIdx) {
			Token currentToken = tokenList[i];
			if(currentToken.Is("$Member")) {
				cmd = cmd + ".";
			}
			cmd = cmd + currentToken.getParsedText();
			if(currentToken.isBeforeWhiteSpace()) {
				beginIdx = i;
				currentToken.setParsedText("$Symbol", cmd, 0, cmd.getlength());
				firstToken = currentToken;
				break;
			}
			i = i + 1;
		}
		isSymbol = true;
	}
	if(defined(DEBUG)) {
		System.p(firstToken);
	}
	if((isSymbol || firstToken.Is("$Symbol")) && SubProc.isCommand(firstToken.getParsedText())) {
		int i = beginIdx + 1;
		while(i < endIdx) {
			if(tokenList[i].Is("()")) {
				return -1;
			}
			i = i + 1;
		}
		String[] args = concatTokenList(tokenList.slice(beginIdx, i));
		if(defined(DEBUG)) {
			System.p(args);
		}
		if(!SubProc.isCommand(args[0])) {
			return -1;
		}
		if(defined(OUTPUT_CMD)) {
			int j = 0;
			stdout.print("execute command: ");
			while(j < args.getlength()) {
				stdout.print(args[j] + " ");
				j = j + 1;
			}
			stdout.println("");
		}
		Token[] subprocTokens = node.getNameSpace().Preprocess(generateSubProcSequence(args, true, ignoreErrorStatus));
		Node sequence = node.ParseNewNode(subprocTokens, 0, subprocTokens.getlength(), false);
		node.AddParsedObject("$SubprocSequence", sequence);
		return i;
	}
	return -1;
}

int getLastIndex(String source) {
	int i = 0;
	String endSymbol = " )}\n";
	while(i < source.getlength()) {
		if(endSymbol.indexOf(source[i]) >= 0) {
			return i;
		}
		i = i + 1;
	}
	return -1;
}

int TokenizeDollarVariable(Token tk, String source) {
	int pos = 1;
	if(source.indexOf("{") == 1) {
		pos = source.indexOf("}");
		if(pos > 2) {
			tk.setParsedText("$DollarVariable", source, 0, pos + 1);
		}
		return pos + 1;
	}
	else if(source.indexOf("(") == 1) {
		pos = source.indexOf(")");
		if(pos > 2) {
			tk.setParsedText("$DollarVariable", source, 0, pos + 1);
		}
		return pos + 1;
	}
	pos = getLastIndex(source);
	if(pos < 0) {
		pos = source.getlength();
	}
	tk.setParsedText("$DollarVariable", source, 0, pos);
	return pos;
}

int ParseDollarVariable(Node node, Symbol sym, Token[] tokenList, int beginIdx, int operatorIdx, int endIdx) {
	NameSpace ns = node.getNameSpace();
	boolean ignoreErrorStatus = false;
	if(defined(DEBUG)) {
		System.p(node);
	}
	String text = tokenList[operatorIdx].getParsedText();
	if(text.startsWith("$(") && text.endsWith(")")) {
		String[] args = text.substring(2, text.getlength() - 1).split(" ");
		if(args[0] == "!") {
			ignoreErrorStatus = true;
			args.removeAt(0);
		}
		if(!SubProc.isCommand(args[0])) {
			return -1;
		}
		if(defined(OUTPUT_CMD)) {
			int j = 0;
			stdout.print("execute command: ");
			while(j < args.getlength()) {
				stdout.print(args[j] + " ");
				j = j + 1;
			}
			stdout.println("");
		}
		Token[] tokens = ns.Preprocess(generateSubProcSequence(args, false, ignoreErrorStatus));
		Node sequence = node.ParseNewNode(tokens, 0, tokens.getlength(), false);
		node.AddParsedObject("$SubprocSequence", sequence);
		return endIdx;
	}
	else if(text.startsWith("${") && text.endsWith("}")) {
		text = text.substring(2, text.getlength() - 1);
		node.AddParsedObject("text", node.ParseNewNode(ns.Preprocess(text), 0, 1, false));
		return endIdx;
	}
	else if(text == "$?") {
		Token methodToken = ns.Preprocess("getExitStatus")[0];
		Node classExpr = new Node(ns);
		classExpr.SetConstValue(ns);
		node.AddParsedObject("class", classExpr);
		node.AddParsedObject("method", methodToken);
		return endIdx;
	}
	text = text.substring(1, text.getlength());
	node.AddParsedObject("text", node.ParseNewNode(ns.Preprocess(text), 0, 1, false));
	return endIdx;
}

Node TypeDollarVariable(Node node, NameSpace ns, Object o) {
	if(defined(DEBUG)) {
		System.p(node);
	}
	if(node.getNode("text") != null) {
		return node.TypeCheckNode("text", o, 0);
	}
	Node classNode = node.getNode("class");
	if(classNode != null) {
		Symbol methodName = node.getToken("method", null).getParsedText();
		return node.newMethodNode(NameSpace, methodName, classNode);
	}
	Node sequence = node.TypeCheckNode("$SubprocSequence", String, TypeCheckPolicy_AllowEmpty);
	return sequence;
}

void defineShellSyntax() {
	CompileAllDefinedMethod();
	Syntax shellSyntax = new Syntax("$Shell");
	shellSyntax.SetParseFunc(Precedence_Statement, 0, ParseShell);
	shellSyntax.SetMetaPattern(true);
	shellSyntax.SetTypeFunc(TypeShell);
	DefineSyntax(shellSyntax);

	Syntax dollarSyntax = new Syntax("$DollarVariable");
	dollarSyntax.SetTokenFunc(36/*$*/, TokenizeDollarVariable);
	dollarSyntax.SetParseFunc(0, 0, ParseDollarVariable);
	dollarSyntax.SetTypeFunc(TypeDollarVariable);
	DefineSyntax(dollarSyntax);
}

defineShellSyntax();
