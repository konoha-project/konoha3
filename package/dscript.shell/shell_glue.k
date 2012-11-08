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
import("jansson");
import("konoha.array");
import("konoha.const");
import("konoha.eval");
import("konoha.object");
import("konoha.file");
import("konoha.global");
import("konoha.string");
import("konoha.eventlistener");
import("posix.path");
import("dscript.subproc");
import("curl");
import("sugar");

int subproc_id = 0;
const DEBUG = false;

void childCallback(Event e) {
	if(DEBUG) {
		System.p("Event      : " + e.getProperty("event"));
		System.p("Pid        : " + e.getInt("Pid"));
		System.p("Command    : " + e.getProperty("Command"));
		System.p("ReturnCode : " + e.getInt("ReturnCode"));
	}
}

@Public void sendHttpEvent(String command, SubProc sp) {
	Json sdata = new Json();
	sdata.setString("event", "ChildProcessExited");
	sdata.setString("Command", command);
	sdata.setInt("ReturnCode", sp.getStatus());
	sdata.setInt("Pid", sp.getPid());
	String req = sdata.dump();
	Curl c = new Curl();
	String url = "http://0.0.0.0:8100";
	c.setOpt(CURLOPT_URL, url);
	c.setOpt(CURLOPT_POSTFIELDS, req);
	c.perform();
}

EventHandler.setCallBack("ChildProcessExited", childCallback);

void parentCallback(Event e) {
	if(DEBUG) {
		System.p("Signal : " + e.getProperty("event"));
	}
}

boolean setDefaultEventCallback() {
	int i = 0;
	while(i < 32/*_POSIX_SIGQUEUE_MAX*/) {
		EventHandler.setCallBack(i, parentCallback);
		i = i + 1;
	}
	return true;
}

setDefaultEventCallback();
HttpEventListener.start("0.0.0.0", 8100);
SignalEventListener.start();

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
	return ret.substr(0, ret.getSize() - 1);
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

String getPipeString(String prevname, String varname, String[] args, int startindex, int argsize) {
	String ret = "";
	if(prevname != "" && !isCommandUseTty(args[startindex])) {
		ret = ret + prevname + ".pipe(" + varname + ", false);\n";
	}
	if(isCommandUseTty(args[startindex])) {
		if(prevname != "") {
			ret = ret + varname + ".bg();\n";
			ret = ret + varname + ".communicate(" + prevname + ".getOutputStream().readAll());\n";
		}
		else {
			ret = ret + varname + ".fg();\n";
		}
	}
	else {
		ret = ret + varname + ".fg();\n";
	}
	ret = ret + "sendHttpEvent(\"" + join(slice(args, startindex, argsize + 1), " ") + "\", " + varname + ");\n";
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
		varname = getVarName();
		ret = ret + "SubProc " + varname + " = new SubProc(\"" + args[startindex] + "\");\n";
		if(argsize > 0) {
			ret = ret + varname + ".setArgumentList(" + slice(args, startindex + 1, argsize) + ");\n";
		}
		if(index >= 0) {
			ret = ret + getPipeString(prevname, varname, args, startindex, argsize);
		}
		else {
			if(redirectindex >= 0) {
				/* redirect */
				String filename = args[redirectindex + 1];
				ret = ret + varname + ".setOutputStream(new FILE(\"" + filename + "\", \"w\"));\n";
			}
			else {
				/* end of pipe */
				ret = ret + varname + ".setOutputStream(stdout);\n";
			}
			ret = ret + getPipeString(prevname, varname, args, startindex, argsize);
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
	if(tokenList.isNull()) {
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

int patternMatchShell(Stmt stmt, int nameid, Token[] tokenList, int beginIdx, int endIdx) {
	Token firstToken = tokenList[beginIdx];
	if(firstToken.isSymbol() && SubProc.isCommand(firstToken.getText())) {
		int i = beginIdx + 1;
		while(i < endIdx) {
			if(tokenList[i].isParenthesis()) {
				return -1;
			}
			i = i + 1;
		}
		return beginIdx;
	}
	return -1;
}

addPatternMatch("$Shell", patternMatchShell);
addStatement("$Shell", statementShell);
addTopLevelStatement("$Shell", statementShell);

syntax $Shell $Token*
