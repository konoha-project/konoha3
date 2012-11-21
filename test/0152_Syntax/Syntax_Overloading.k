// written by kimio kuramitsu

import("konoha.desugar");

syntax "for" "(" $Stmt ";" $Expr ";" $Stmt ")" $Block;
syntax "for" "(" $Type $Expr "in" $Expr ")" $Block;

