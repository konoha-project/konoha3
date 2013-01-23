// written by kimio kuramitsu

import("MiniKonoha.Syntax");

Node TypeCheckTest(Node node, NameSpace ns, Object type) {
	node.message(3, "Info message");
	node.done();
	return node;
}

void main(){
	Syntax syntax = new Syntax("test");
	syntax.SetParseFunc(Precedence_Statement, 0, GetPatternParseFunc());
	syntax.SetPattern("\"test\"");
	syntax.SetTypeFunc(TypeCheckTest);
	DefineSyntax(syntax);
}
main();

test;

