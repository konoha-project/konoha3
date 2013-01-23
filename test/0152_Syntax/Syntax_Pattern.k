// written by kimio kuramitsu

import("MiniKonoha.Syntax");

void main(){
	Syntax syntax = new Syntax("$test");
	syntax.setParseFunc(Precedence_Statement, 0, GetPatternParseFunc());
	syntax.SetPattern("$CDecl $Type ClassName: $Type \".\" $Symbol [$Text] $Param $CDeclBody");
	DefineSyntax(syntax);
}
main();
