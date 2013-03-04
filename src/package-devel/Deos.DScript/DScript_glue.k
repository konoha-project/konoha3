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

Import("Deos.DCaseCloud");
Import("Dse.Control");
Import("JavaScript.Array");
Import("JavaScript.String");
Import("Lib.Openssl");
Import("MiniKonoha.Eval");
Import("MiniKonoha.NameSpace");
Import("MiniKonoha.Syntax");
Import("Syntax.CStyleWhile");
Import("Syntax.GlobalVariable");
Import("Syntax.JavaStyleClass");
Import("Syntax.Null");
Import("Type.Json");

void CheckDControlIsRunning() {
	if(!DControl.start(8090)) {
		System.p("Master DSE is not running.");
		assert(false);
	}
}

CheckDControlIsRunning();

// --------------------------------------------------------------------------
// [global variables]

class TaskData {
	String target;
	String body;

	TaskData(String t, String b) {
		this.target = t;
		this.body = b;
	}
}

Json _names = new Json();
Json _tasks = new Json();
API repository = new API("dcasedb", "dcasedb"); // TODO
Json[] _context = [];
String[] _target = [];
boolean[] _compiled = [];
String[] _argument = [];

String GetName(String hash) {
	return global._names.getString(hash);
}

void SetName(String hash, String funcname) {
	global._names.setString(hash, funcname);
}

void AddTask(String key, String target, String task) {
	if(global._tasks.hasKey(key)) {
		global._tasks.setString(key, global._tasks.getString(key) + "," + target + ":" + task);
	}
	else {
		global._tasks.setString(key, target + ":" + task);
	}
}

TaskData[] GetTaskList(String key) {
	if(global._tasks.getString(key) == "") {
		return [];
	}
	String[] data = global._tasks.getString(key).split(",");
	int i = 0;
	TaskData[] tdata = [];
	while(i < data.getlength()) {
		String[] t_b = data[i].split(":");
		tdata.add(new TaskData(t_b[0], t_b[1]));
		i = i + 1;
	}
	return tdata;
}

String String.CreateHash() {
	MD5 md5 = new MD5();
	md5.update(this);
	return "f" + md5.final();
}

boolean DefinedContext(String hash) {
	return global._names.hasKey(hash.CreateHash());
}

String GetTarget() {
	return global._target[0];
}

void PushTarget(String target) {
	global._target.add(target);
}

String PopTarget() {
	return global._target.removeAt(0);
}

boolean IsArgumentCompiled() {
	return global._compiled[global._compiled.getlength() - 1];
}

void SetArgumentCompiled() {
	global._compiled[global._compiled.getlength() - 1] = true;
}

void PushArgumentCompiled(boolean compiled) {
	global._compiled.add(false);
}

boolean PopArgumentCompiled() {
	return global._compiled.pop();
}

String GetCurrentArgument() {
	return global._argument[global._argument.getlength() - 1];
}

void PushCurrentArgument(String name) {
	global._argument.add(name);
}

String PopCurrentArgument() {
	return global._argument.pop();
}

String EscapeBody(String body) {
	return body.replace("\n", "\\n").replace("\"", "\\\"").replace("\t", "");
}

String CreateDseCode(String goal, String target, String body) {
	String ret = "DseGroup group = new DseGroup();\n";
	String[] hosts = target.split(",");
	int i = 0;
	while(i < hosts.getlength()) {
		String[] host_port = hosts[i].split(":");
		ret = ret + "group.appendDse(\"" + host_port[0] + "\", " + host_port[1] + ");\n";
		i = i + 1;
	}
	ret = ret + "group.appendDTask(DTask.CreateFromText(\"";
	ret = ret + "Import(\\\"Syntax.ShellStyle\\\");\\n";
	ret = ret + EscapeBody(body);
	ret = ret + "\"));\n";
	ret = ret + "AppendAdditionalTasks(\"" + goal + "\", group);\n";
	ret = ret + "Json[] result = group.dispatchDTasks();\n";
	ret = ret + "if(result.get(0).getString(\"stdout\").startsWith(\" - (error) \")) { return false; }\n";
	ret = ret + "if(result.get(0).getString(\"stdout\").indexOf(\"Exception\") >= 0) { System.p(result.get(0).getString(\"stdout\"));\nreturn false; }\n";
	ret = ret + "return (result.get(0).getInt(\"code\") == 0);\n";
	if(defined(DEBUG)) {
		System.p(ret);
	}
	return ret;
}

// --------------------------------------------------------------------------
// [class definitions]

class DCaseContext {
	@Static String Get(String key) {
		//System.p("Get Context '" + key + "'");
		int i = global._context.getlength() - 1;
		while(i >= 0) {
			String val = global._context[i].getString(key);
			if(val != "") {
				return val;
			}
			i = i - 1;
		}
		return "";
	}
	@Static void Push(Json context) {
		global._context.add(context);
	}
	@Static Json Pop() {
		return global._context.pop();
	}
}

@Public void AppendAdditionalTasks(String goal, DseGroup group) {
	int i = 0;
	TaskData[] tlist = GetTaskList(goal);
	while(i < tlist.getlength()) {
		String[] hosts = DCaseContext.Get(tlist[i].target).split(",");
		int j = 0;
		while(j < hosts.getlength()) {
			String[] host_port = hosts[i].split(":");
			Dse dse = new Dse(host_port[0], host_port[1].toint());
			dse.appendDTask(DTask.CreateFromText("Import(\"Syntax.ShellStyle\");" + tlist[i].body));
			group.appendDse(dse);
			j = j + 1;
		}
		i = i + 1;
	}
}

CompileAllDefinedMethod();

class DCaseAssurance {
	@Static boolean TypeCheck(String goal) {
		//System.p("Typecheck Assurance '" + goal + "'");
		Json root = global.repository.Search(goal);
		if(root.getSize() > 0) {
			int i = 0;
			while(i < root.getSize()) {
				Json json = root[i];
				int a_id = json.getString("argument_id").toint();
				DCaseArgument arg = global.repository.GetArgument(a_id);
				Json ctx = arg.GetRoot().GetContext();
				if(ctx.getSize() > 0) {
					//System.p(ctx);
					DCaseContext.Push(ctx);
				}
				i = i + 1;
			}
			return true;
		}
		root = global.repository.Search(goal, "Evidence");
		if(root.getSize() > 0) {
			return true;
		}
		return false;
	}
}

class DArgument {
	@Static boolean TypeCheck(String goal, String context) {
		//System.p("Typecheck Argument '" + goal + "' with Context '" + context + "'");
		Json root = global.repository.Search(goal);
		if(root.getSize() > 0) {
			int i = 0;
			while(i < root.getSize()) {
				Json json = root[i];
				int a_id = json.getString("argument_id").toint();
				DCaseArgument arg = global.repository.GetArgument(a_id);
				Json ctx = arg.GetRoot().GetContext();
				if(ctx.getSize() > 0) {
					//System.p(ctx);
					DCaseContext.Push(ctx);
				}
				i = i + 1;
			}
			return true;
		}
		return true;
	}
}

// --------------------------------------------------------------------------
// [API]

@Override @Public boolean String.startsWith(String str) {
	return StringUtil.startsWith(this, str, 0);
}

@Override @Public boolean String.endsWith(String str) {
	return StringUtil.endsWith(this, str);
}

String String.Decode() {
	return this.replace("\\\"", "\"").replace("\\n", "\n");
}

String String.Encode() {
	return this.replace("\"", "\\\"").replace("\n", "\\n");
}

String Node.GetText(Symbol key) {
	return this.GetToken(key, null).GetParsedText().Decode();
}

Token Node.PreprocessText(String text) {
	return this.getNameSpace().Preprocess("\"" + text.Encode() + "\"")[0];
}

Token NewBlockWithReturnValue(Node node, Token block) {
	String innerText = block.GetParsedText();
	//innerText = innerText + "\treturn true;";
	return node.PreprocessText(innerText);
}

String GetJoinedString(Token[] tokenList, int start, int end) {
	String ret = tokenList[start].GetParsedText();
	int i = start + 1;
	while(i < end) {
		// FIXME: A token after '-' always return true in IsBeforeWhiteSpace()
		if(tokenList[i].IsBeforeWhiteSpace() && tokenList[i - 1].GetParsedText() != "-") {
			ret = ret + " ";
		}
		ret = ret + tokenList[i].GetParsedText();
		i = i + 1;
	}
	return ret;
}

Node NameSpace.CreateFuncNode(String methodName) {
	Node node = new Node(this);
	node.AddParsedObject("func", node.ParseNewNode(this.Preprocess(methodName), 0, 1, false));
	return node.TypeCheckNode("func", Func[boolean], 0);
}

Node NewNameSpaceConst(NameSpace ns) {
	Node classNode = new Node(ns);
	return classNode.SetConstValue(ns);
}

boolean IsAnnotation(String source) {
	String[] annotations = ["@Public", "@Const", "@Static", "@Virtual", "@Final", "@Override", "@Restricted"];
	int i = 0;
	while(i < annotations.getlength()) {
		if(source.startsWith(annotations[i])) {
			return true;
		}
		i = i + 1;
	}
	return false;
}

String GetAssurePrefix(String source) {
	String[] tokens = source.split(" ");
	if(tokens.getlength() > 1 && tokens[1] == "assure") {
		return tokens[0].substring(1);
	}
	return "";
}

boolean IsStrategyRule(String text) {
	return (text == "pre" || text == "post" || text == "concurrent");
}

int GetLastIndex(String source) {
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

// --------------------------------------------------------------------------
// [Syntax API]

int TokenizeAt(Token tk, String source) {
	int pos = 1;
	//System.p(source);
	if(IsAnnotation(source)) {
		return -1;
	}
	String prefix = GetAssurePrefix(source);
	if(prefix != "") {
		RegExp r1 = new RegExp("^@.*{\n", "g");
		RegExp r2 = new RegExp("{", "g");
		RegExp r3 = new RegExp("}", "g");
		if(r1.exec(source).getlength() == 1 && r2.exec(source).getlength() == r3.exec(source).getlength()) {
			PushTarget(prefix);
		}
		return prefix.getlength() + 1;
	}
	if(source.indexOf("{") == 1) {
		pos = source.indexOf("}");
		if(pos > 2) {
			tk.setParsedText("$AtVariable", source, 0, pos + 1);
		}
		return pos + 1;
	}
	else if(source.indexOf("[") == 1) {
		pos = source.indexOf("]");
		if(pos > 2) {
			tk.setParsedText("$AtVariable", source, 0, pos + 1);
		}
		return pos + 1;
	}
	pos = GetLastIndex(source);
	if(pos < 0) {
		pos = source.getlength();
	}
	tk.setParsedText("$AtVariable", source, 0, pos);
	return pos;
}

int ParseAt(Node node, Symbol sym, Token[] tokenList, int beginIdx, int operatorIdx, int endIdx) {
	String text = tokenList[operatorIdx].GetParsedText();
	if(text.startsWith("@{") && text.endsWith("}")) {
		text = text.substring(2, text.getlength() - 1);
		node.AddParsedObject("ContextKey", node.PreprocessText(text));
		return endIdx;
	}
	else if(text.startsWith("@[") && text.endsWith("]")) {
		text = text.substring(2, text.getlength() - 1);
		node.AddParsedObject("AssureKey", node.PreprocessText(text.CreateHash()));
		return endIdx;
	}
	text = text.substring(1, text.getlength());
	node.AddParsedObject("ContextKey", node.PreprocessText(text));
	return endIdx;
}

Node TypeAt(Node node, NameSpace ns, Object o) {
	String contextKey = node.GetText("ContextKey");
	if(contextKey != "") {
		String contextValue = DCaseContext.Get(contextKey);
		if(contextValue == "") {
			return node.Message(ErrTag, "Undefined Context '" + contextKey + "'.");
		}
		return node.SetConstValue(contextValue);
	}
	String methodName = node.GetText("AssureKey");
	Node retNode = null;
	Node classNode = NewNameSpaceConst(ns);
	if(DefinedContext(methodName)) {
		Node funcNode = ns.CreateFuncNode(methodName);
		retNode = node.newMethodNode(NameSpace, methodName.CreateHash(), classNode, funcNode);
	}
	else {
		retNode = node.newMethodNode(NameSpace, methodName, classNode);
	}
	if(retNode == null) {
		retNode = node.Message(ErrTag, "Undefined Assurance '" + GetName(methodName) + "'.");
	}
	return retNode;
}

int ParseAssure(Node node, Symbol sym, Token[] tokenList, int beginIdx, int operatorIdx, int endIdx) {
	if(endIdx - beginIdx <= 2 || !tokenList[endIdx - 1].Is("{}")) {
		return -1;
	}
	String nodeText = GetJoinedString(tokenList, beginIdx + 1, endIdx - 1);
	Token block = NewBlockWithReturnValue(node, tokenList[endIdx - 1]);
	String funcName = nodeText.CreateHash();
	node.AddParsedObject("goal", node.PreprocessText(funcName));
	node.AddParsedObject("body", block);
	return endIdx;
}

Node TypeAssure(Node node, NameSpace ns, Object o) {
	String goal = node.GetText("goal");
	String body = node.GetText("body");
	if(DCaseAssurance.TypeCheck(GetName(goal))) {
		node.AddParsedObject("target", node.PreprocessText(GetTarget()));
		String target = DCaseContext.Get(node.GetText("target"));
		if(target == "") {
			return node.Message(ErrTag, "Undefined Context '" + target + "'.");
		}
		String evalText = "@Public boolean " + goal + "() {";
		evalText = evalText + CreateDseCode(goal, target, body) + "}";
		boolean ret = ns.eval(evalText);
		//TODO:evalate context correctly
		//DCaseContext.Pop();
		PopTarget();
		return node.SetConstValue(ret);
	}
	return node.Message(ErrTag, "Assurance '" + GetName(goal) + "' does not exist in D-Case Cloud.");
}

int ParseArgue(Node node, Symbol sym, Token[] tokenList, int beginIdx, int operatorIdx, int endIdx) {
	//if(endIdx - beginIdx <= 2 || !tokenList[endIdx - 1].Is("{}") || !tokenList[endIdx - 2].Is("()")) {
	//	return -1;
	//}
	//Token[] groupTokenList = tokenList[endIdx - 2].GetGroupTokenList();
	//if(groupTokenList.getlength() < 3) {
	//	return -1;
	//}
	//String contextText = GetJoinedString(groupTokenList, 1, groupTokenList.getlength() - 2);

	//// FIXME: IsBeforeWhiteSpace does not work correctly when applying to
	////        last element of GroupTokenList.
	//contextText = contextText + " " + groupTokenList[groupTokenList.getlength() - 2].GetParsedText();
	if(endIdx - beginIdx <= 2 || !tokenList[endIdx - 1].Is("{}")) {
		return -1;
	}
	String contextText = GetJoinedString(tokenList, beginIdx + 1, endIdx - 1);

	node.AddParsedObject("context", node.PreprocessText(contextText.CreateHash()));
	Token block = NewBlockWithReturnValue(node, tokenList[endIdx - 1]);
	node.AddParsedObject("body", block);
	return endIdx;
}

Node TypeArgue(Node node, NameSpace ns, Object o) {
	String body = node.GetText("body");
	String context = node.GetText("context");
	if(DArgument.TypeCheck("D-Script must work", GetName(context))) {
		PushCurrentArgument(context);
		String funcName = context.CreateHash();
		SetName(funcName, GetName(context));
		String evalText = "@Public boolean " + funcName + "(Func[boolean] dscript) {";
		evalText = evalText + body + "\treturn true;\n}";
		boolean ret = ns.eval(evalText);

		//Node funcNode = ns.CreateFuncNode(context);
		//Node classNode = NewNameSpaceConst(ns);
		//Node methodNode = node.newMethodNode(NameSpace, funcName, classNode, funcNode);
		//methodNode.SetType(KNode_MethodCall);
		//TODO:evalate context correctly
		//DCaseContext.Pop();
		//return methodNode;
		return node.SetConstValue(ret);
	}
	return node.Message(ErrTag, "Assurance 'D-Script must work' with Context '" + GetName(context) + "' does not exist in D-Case Cloud.");
}

int ParseStrategy(Node node, Symbol sym, Token[] tokenList, int beginIdx, int operatorIdx, int endIdx) {
	if(endIdx - beginIdx <= 2 || !tokenList[endIdx - 1].Is("{}")) {
		return -1;
	}
	String strategyText = GetJoinedString(tokenList, beginIdx + 1, endIdx - 1);
	if(IsStrategyRule(strategyText)) {
		Node[] innerBlocks = node.ParseNewNode(tokenList, endIdx - 1, endIdx, false).GetNodeList();
		node.AddParsedObject("rule", node.PreprocessText(strategyText));
		node.AddParsedObject("size", node.PreprocessText(innerBlocks.getlength().toString()));
		int i = 0;
		while(i < innerBlocks.getlength()) {
			node.AddParsedObject("f" + i, innerBlocks[i]);
			i = i + 1;
		}
		node.AddParsedObject("func", node.ParseNewNode(node.getNameSpace().Preprocess("dscript()"), 0, 2, false));
		PushArgumentCompiled(false);
		return endIdx;
	}
	return -1;
}

Node TypeStrategy(Node node, NameSpace ns, Object o) {
	Node classNode = NewNameSpaceConst(ns);
	Node argumentFuncNode = node.TypeCheckNode("func", boolean, 0);
	int size = node.GetText("size").toint();
	int i = 0;
	String[] funcNames = [];
	while(i < size) {
		String key = "f" + i;
		node.TypeCheckNode(key, boolean, 0);
		funcNames.add(node.GetNode(key).GetText("goal"));
		i = i + 1;
	}
	Node newNode = new Node(ns);
	String rule = node.GetText("rule");
	if(rule == "pre") {
		i = 0;
		while(i < funcNames.getlength()) {
			newNode.AddNode(node.newMethodNode(NameSpace, funcNames[i], classNode));
			i = i + 1;
		}
		if(!IsArgumentCompiled()) {
			newNode.AddNode(argumentFuncNode);
			SetArgumentCompiled();
		}
	}
	else if(rule == "post") {
		if(!IsArgumentCompiled()) {
			newNode.AddNode(argumentFuncNode);
			SetArgumentCompiled();
		}
		i = 0;
		while(i < funcNames.getlength()) {
			newNode.AddNode(node.newMethodNode(NameSpace, funcNames[i], classNode));
			i = i + 1;
		}
	}
	else if(rule == "concurrent") {
		if(!IsArgumentCompiled()) {
			newNode.AddNode(argumentFuncNode);
			SetArgumentCompiled();
		}
		i = 0;
		while(i < funcNames.getlength()) {
			Node fnode = node.GetNode("f" + i);
			AddTask(GetCurrentArgument(), fnode.GetText("target"), fnode.GetText("body").replace("\t", ""));
			i = i + 1;
		}
	}
	newNode.SetType(KNode_Block);
	return newNode;
}

// --------------------------------------------------------------------------

void DefineDscriptSyntax() {
	Syntax atSyntax = new Syntax("$AtVariable");
	atSyntax.SetTokenFunc(64/*@*/, TokenizeAt);
	atSyntax.SetParseFunc(0, 0, ParseAt);
	atSyntax.SetTypeFunc(TypeAt);
	DefineSyntax(atSyntax);

	Syntax assureSyntax = new Syntax("assure");
	assureSyntax.SetParseFunc(Precedence_Statement, 0, ParseAssure);
	assureSyntax.SetTypeFunc(TypeAssure);
	DefineSyntax(assureSyntax);

	Syntax argueSyntax = new Syntax("argue");
	argueSyntax.SetParseFunc(Precedence_Statement, 0, ParseArgue);
	argueSyntax.SetTypeFunc(TypeArgue);
	DefineSyntax(argueSyntax);

	Syntax strategySyntax = new Syntax("strategy");
	strategySyntax.SetParseFunc(Precedence_Statement, 0, ParseStrategy);
	strategySyntax.SetTypeFunc(TypeStrategy);
	DefineSyntax(strategySyntax);
}

DefineDscriptSyntax();
