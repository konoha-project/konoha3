// written by kimio kuramitsu

import("MiniKonoha.Syntax");

void main(){
	Syntax syntax = new Syntax("for");
	syntax.setParseFunc(Precedence_Statement, 0, GetPatternParseFunc());
	syntax.SetPattern("\"for\" \"(\" $Stmt \";\" $Expr \";\" $Stmt \")\" $Block");
	DefineSyntax(syntax);
	
	Syntax syntax2 = new Syntax("for");
	syntax2.setParseFunc(Precedence_Statement, 0, GetPatternParseFunc());
	syntax2.SetPattern("\"for\" \"(\" $Type $Expr \"in\" $Expr \")\" $Block");
	DefineSyntax(syntax2);

}
main();

