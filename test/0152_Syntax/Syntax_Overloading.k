// written by kimio kuramitsu

import("MiniKonoha.Syntax");

syntax "for" "(" $Stmt ";" $Expr ";" $Stmt ")" $Block;
syntax "for" "(" $Type $Expr "in" $Expr ")" $Block;

