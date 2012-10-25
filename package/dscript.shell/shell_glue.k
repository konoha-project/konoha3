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
import("konoha.array");
import("konoha.null");
import("konoha.const");
import("konoha.file");
import("konoha.new");
import("posix.path");
import("dscript.subproc");
import("konoha.global");
import("sugar");

int subproc_id = 0;
const DEBUG = false;

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
	String ret = "s" + global.subproc_id;
	global.subproc_id = global.subproc_id + 1;
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

String[] slice(String[] a, int s, int length) {
	String[] ret = [];
	int i = 0;
	while(i < length) {
		ret.add(a[s + i]);
		i = i + 1;
	}
	return ret;
}

String generateSubProcSequence(String[] args) {
	if(DEBUG) {
		System.p(args);
	}
	String ret = "";
	int startindex = 0;
	int index = 0;
	String varname = "";
	String prevname = "";
	int argsize = 0;
	int redirectindex = -1;
	while(true) {
		/* pipe */
		index = indexOf(args, "|", startindex);
		if(index < 0) {
			redirectindex = indexOf(args, ">", startindex);
			if(redirectindex >= 0) {
				argsize = redirectindex - startindex - 1;
			}
			else {
				argsize = args.getSize() - startindex - 1;
			}
		}
		else {
			argsize = index - startindex - 1;
		}
		if(DEBUG) {
			System.p(startindex); System.p(index); System.p(argsize);
			System.p(redirectindex);
		}
		varname = getVarName();
		ret = ret + "SubProc " + varname + " = new SubProc(\"" + args[startindex] + "\");\n";
		if(argsize > 0) {
			ret = ret + varname + ".setArgumentList(" + slice(args, startindex + 1, argsize) + ");\n";
		}
		if(index >= 0) {
			if(prevname != "") {
				ret = ret + prevname + ".pipe(" + varname + ", false);\n";
			}
			ret = ret + varname + ".fg();\n";
		}
		else {
			if(redirectindex >= 0) {
				String filename = args[redirectindex + 1];
				ret = ret + varname + ".setOutputStream(new FILE(\"" + filename + "\", \"w\"));\n";
			}
			else {
				ret = ret + varname + ".setOutputStream(stdout);\n";
			}
			if(prevname != "") {
				ret = ret + prevname + ".pipe(" + varname + ", false);\n";
			}
			ret = ret + varname + ".fg();\n";
			break;
		}
		prevname = varname;
		startindex = index + 1;
	}
	if(DEBUG) {
		System.p(ret);
	}
	return ret;
}

boolean statementShell(Stmt stmt, Gamma gma) {
	Token[] tokenList = stmt.getTokenList("$Token", null);
	if(tokenList == null) {
		return false;
	}
	String[] args = concatTokenList(tokenList);
	Block block = stmt.newBlock(generateSubProcSequence(args));
	boolean ret = block.tyCheckAll(gma);
	if(!ret) {
		return false;
	}
	stmt.setBlock("$Block", block);
	stmt.setType(TSTMT_BLOCK);
	return ret;
}

boolean checkExecutablePath(String dir, String cmd) {
	DIR d = System.opendir(dir);
	String binname = d.readFileName();
	while(binname != null) {
		binname = d.readFileName();
		if(binname == cmd) return true;
	}
	return false;
}

boolean isCommand(String cmd) {
	String[] dirs = ["/bin", "/sbin", "/usr/bin", "/usr/sbin"];
	int i = 0;
	while(i < dirs.getSize()) {
		if(checkExecutablePath(dirs[i], cmd)) {
			return true;
		}
		i = i + 1;
	}
	return false;
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
